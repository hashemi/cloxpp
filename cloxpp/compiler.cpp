//
//  compiler.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-02.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "compiler.hpp"

void Compiler::addLocal(const std::string& name) {
    if (locals.size() == UINT8_COUNT) {
        parser->error("Too many local variables in function.");
        return;
    }
    locals.emplace_back(Local(name, -1));
}

void Compiler::declareVariable(const std::string& name) {
    if (scopeDepth == 0) return;
    
    for (long i = locals.size() - 1; i >= 0; i--) {
        if (locals[i].depth != -1 && locals[i].depth < scopeDepth) break;
        if (locals[i].name == name) {
            parser->error("Variable with this name already declared in this scope.");
        }
    }
    
    addLocal(name);
}

void Compiler::markInitialized() {
    if (scopeDepth == 0) return;
    locals.back().depth = scopeDepth;
}

int Compiler::resolveLocal(const std::string& name) {
    for (long i = locals.size() - 1; i >=0; i--) {
        if (locals[i].name == name) {
            if (locals[i].depth == -1) {
                parser->error("Cannot read local variable in its own initializer.");
            }
            return static_cast<int>(i);
        }
    }
    
    return -1;
}

void Compiler::beginScope() { scopeDepth++; }

void Compiler::endScope() {
    scopeDepth--;
    while (!locals.empty() && locals.back().depth > scopeDepth) {
        parser->emit(OpCode::POP);
        locals.pop_back();
    }
}

bool Compiler::isLocal() {
    return scopeDepth > 0;
}

Parser::Parser(const std::string& source) :
    previous(Token(TokenType::_EOF, source, 0)),
    current(Token(TokenType::_EOF, source, 0)),
    scanner(Scanner(source)),
    hadError(false), panicMode(false),
    compiler(this, TYPE_SCRIPT)
{
    advance();
}

bool Parser::compile() {
    while (!match(TokenType::_EOF)) {
        declaration();
    }
    endCompiler();
    return !hadError;
}

void Parser::advance() {
    previous = current;
    
    while (true) {
        current = scanner.scanToken();
        if (current.type() != TokenType::ERROR) break;
        
        errorAtCurrent(std::string(current.text()));
    }
}

void Parser::consume(TokenType type, const std::string& message) {
    if (current.type() == type) {
        advance();
        return;
    }
    
    errorAtCurrent(message);
}

bool Parser::check(TokenType type) {
    return current.type() == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

void Parser::emit(uint8_t byte) {
    currentChunk().write(byte, previous.line());
}

void Parser::emit(OpCode op) {
    currentChunk().write(op, previous.line());
}

void Parser::emit(OpCode op, uint8_t byte) {
    emit(op);
    emit(byte);
}

void Parser::emit(OpCode op1, OpCode op2) {
    emit(op1);
    emit(op2);
}

void Parser::emitLoop(int loopStart) {
    emit(OpCode::LOOP);
    
    int offset = currentChunk().count() - loopStart + 2;
    if (offset > UINT16_MAX) { error("Loop body too large."); }
    
    emit((offset >> 8) & 0xff);
    emit(offset & 0xff);
}

int Parser::emitJump(OpCode op) {
    emit(op);
    emit(0xff);
    emit(0xff);
    return currentChunk().count() - 2;
}

void Parser::emitReturn() {
    emit(OpCode::RETURN);
}

uint8_t Parser::makeConstant(Value value) {
    auto constant = currentChunk().addConstant(value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }
    
    return (uint8_t)constant;
}

void Parser::emitConstant(Value value) {
    emit(OpCode::CONSTANT, makeConstant(value));
}

void Parser::patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk().count() - offset - 2;
    
    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }
    
    currentChunk().setCode(offset, (jump >> 8) & 0xff);
    currentChunk().setCode(offset + 1, jump & 0xff);
}

void Parser::endCompiler() {
    emitReturn();
    
#ifdef DEBUG_PRINT_CODE
    if (!hadError) {
        currentChunk().disassemble("code");
    }
#endif
}

void Parser::binary(bool canAssign) {
    // Remember the operator.
    auto operatorType = previous.type();
    
    // Compile the right operand.
    auto rule = getRule(operatorType);
    parsePrecedence(Precedence(static_cast<int>(rule.precedence) + 1));
    
    // Emit the operator instruction.
    switch (operatorType) {
        case TokenType::BANG_EQUAL:    emit(OpCode::EQUAL, OpCode::NOT); break;
        case TokenType::EQUAL_EQUAL:   emit(OpCode::EQUAL); break;
        case TokenType::GREATER:       emit(OpCode::GREATER); break;
        case TokenType::GREATER_EQUAL: emit(OpCode::LESS, OpCode::NOT); break;
        case TokenType::LESS:          emit(OpCode::LESS); break;
        case TokenType::LESS_EQUAL:    emit(OpCode::GREATER, OpCode::NOT); break;
        case TokenType::PLUS:          emit(OpCode::ADD); break;
        case TokenType::MINUS:         emit(OpCode::SUBTRACT); break;
        case TokenType::STAR:          emit(OpCode::MULTIPLY); break;
        case TokenType::SLASH:         emit(OpCode::DIVIDE); break;
        default:
            return; // Unreachable.
    }
}

void Parser::literal(bool canAssign) {
    switch (previous.type()) {
        case TokenType::FALSE: emit(OpCode::FALSE); break;
        case TokenType::NIL:   emit(OpCode::NIL); break;
        case TokenType::TRUE:  emit(OpCode::TRUE); break;
        default:
            return; // Unreachable.
    }
}

void Parser::grouping(bool canAssign) {
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Parser::number(bool canAssign) {
    Value value = std::stod(std::string(previous.text()));
    emitConstant(value);
}

void Parser::or_(bool canAssign) {
    int elseJump = emitJump(OpCode::JUMP_IF_FALSE);
    int endJump = emitJump(OpCode::JUMP);
    
    patchJump(elseJump);
    emit(OpCode::POP);
    
    parsePrecedence(Precedence::OR);
    patchJump(endJump);
}

void Parser::string(bool canAssign) {
    auto str = previous.text();
    str.remove_prefix(1);
    str.remove_suffix(1);
    emitConstant(std::string(str));
}

void Parser::namedVariable(const std::string& name, bool canAssign) {
    OpCode getOp, setOp;
    auto arg = compiler.resolveLocal(name);
    if (arg != -1) {
        getOp = OpCode::GET_LOCAL;
        setOp = OpCode::SET_LOCAL;
    } else {
        arg = identifierConstant(name);
        getOp = OpCode::GET_GLOBAL;
        setOp = OpCode::SET_GLOBAL;
    }
    
    if (canAssign && match(TokenType::EQUAL)) {
        expression();
        emit(setOp, (uint8_t)arg);
    } else {
        emit(getOp, (uint8_t)arg);
    }
}

void Parser::variable(bool canAssign) {
    namedVariable(std::string(previous.text()), canAssign);
}

void Parser::and_(bool canAssign) {
    int endJump = emitJump(OpCode::JUMP_IF_FALSE);
    
    emit(OpCode::POP);
    parsePrecedence(Precedence::AND);
    
    patchJump(endJump);
}

void Parser::unary(bool canAssign) {
    auto operatorType = previous.type();
    
    // Compile the operand.
    parsePrecedence(Precedence::UNARY);
    
    // Emit the operator instruciton.
    switch (operatorType) {
        case TokenType::BANG: emit(OpCode::NOT); break;
        case TokenType::MINUS: emit(OpCode::NEGATE); break;
            
        default:
            return; // Unreachable.
    }
}

ParseRule& Parser::getRule(TokenType type) {
    auto grouping = [this](bool canAssign) { this->grouping(canAssign); };
    auto unary = [this](bool canAssign) { this->unary(canAssign); };
    auto binary = [this](bool canAssign) { this->binary(canAssign); };
    auto number = [this](bool canAssign) { this->number(canAssign); };
    auto string = [this](bool canAssign) { this->string(canAssign); };
    auto literal = [this](bool canAssign) { this->literal(canAssign); };
    auto variable = [this](bool canAssign) { this->variable(canAssign); };
    auto and_ = [this](bool canAssign) { this->and_(canAssign); };
    auto or_ = [this](bool canAssign) { this->or_(canAssign); };
    
    static ParseRule rules[] = {
        { grouping,    nullptr,    Precedence::CALL },       // TOKEN_LEFT_PAREN
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_RIGHT_PAREN
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_LEFT_BRACE
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_RIGHT_BRACE
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_COMMA
        { nullptr,     nullptr,    Precedence::CALL },       // TOKEN_DOT
        { unary,       binary,     Precedence::TERM },       // TOKEN_MINUS
        { nullptr,     binary,     Precedence::TERM },       // TOKEN_PLUS
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_SEMICOLON
        { nullptr,     binary,     Precedence::FACTOR },     // TOKEN_SLASH
        { nullptr,     binary,     Precedence::FACTOR },     // TOKEN_STAR
        { unary,       nullptr,    Precedence::NONE },       // TOKEN_BANG
        { nullptr,     binary,     Precedence::EQUALITY },   // TOKEN_BANG_EQUAL
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_EQUAL
        { nullptr,     binary,     Precedence::EQUALITY },   // TOKEN_EQUAL_EQUAL
        { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_GREATER
        { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_GREATER_EQUAL
        { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_LESS
        { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_LESS_EQUAL
        { variable,    nullptr,    Precedence::NONE },       // TOKEN_IDENTIFIER
        { string,      nullptr,    Precedence::NONE },       // TOKEN_STRING
        { number,      nullptr,    Precedence::NONE },       // TOKEN_NUMBER
        { nullptr,     and_,       Precedence::AND },        // TOKEN_AND
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_CLASS
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_ELSE
        { literal,     nullptr,    Precedence::NONE },       // TOKEN_FALSE
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_FUN
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_FOR
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_IF
        { literal,     nullptr,    Precedence::NONE },       // TOKEN_NIL
        { nullptr,     or_,        Precedence::OR },         // TOKEN_OR
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_PRINT
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_RETURN
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_SUPER
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_THIS
        { literal,     nullptr,    Precedence::NONE },       // TOKEN_TRUE
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_VAR
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_WHILE
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_ERROR
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_EOF
    };
    
    return rules[static_cast<int>(type)];
}

void Parser::parsePrecedence(Precedence precedence) {
    advance();
    auto prefixRule = getRule(previous.type()).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }
    
    auto canAssign = precedence <= Precedence::ASSIGNMENT;
    prefixRule(canAssign);
    
    while (precedence <= getRule(current.type()).precedence) {
        advance();
        auto infixRule = getRule(previous.type()).infix;
        infixRule(canAssign);
    }
    
    if (canAssign && match(TokenType::EQUAL)) {
        error("Invalid assignment target.");
        expression();
    }
}

int Parser::identifierConstant(const std::string& name) {
    return makeConstant(name);
}

uint8_t Parser::parseVariable(const std::string& errorMessage) {
    consume(TokenType::IDENTIFIER, errorMessage);
    
    compiler.declareVariable(std::string(previous.text()));
    if (compiler.isLocal()) return 0;
    
    return identifierConstant(std::string(previous.text()));
}

void Parser::defineVariable(uint8_t global) {
    if (compiler.isLocal()) {
        compiler.markInitialized();
        return;
    }
    
    emit(OpCode::DEFINE_GLOBAL, global);
}

void Parser::expression() {
    parsePrecedence(Precedence::ASSIGNMENT);
}

void Parser::block() {
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::_EOF)) {
        declaration();
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Parser::varDeclaration() {
    auto global = parseVariable("Expect variable name.");

    if (match(TokenType::EQUAL)) {
        expression();
    } else {
        emit(OpCode::NIL);
    }
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    
    defineVariable(global);
}

void Parser::expressionStatement() {
    expression();
    emit(OpCode::POP);
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
}

void Parser::forStatement() {
    compiler.beginScope();
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(TokenType::VAR)) {
        varDeclaration();
    } else if (match(TokenType::SEMICOLON)) {
        // no initializer.
    } else {
        expressionStatement();
    }
    
    int loopStart = currentChunk().count();
    
    int exitJump = -1;
    if (!match(TokenType::SEMICOLON)) {
        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
        
        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OpCode::JUMP_IF_FALSE);
        emit(OpCode::POP);
    }

    if (!match(TokenType::RIGHT_PAREN)) {
        int bodyJump = emitJump(OpCode::JUMP);
        
        int incrementStart = currentChunk().count();
        expression();
        emit(OpCode::POP);
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
        
        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }
    
    statement();
    
    emitLoop(loopStart);
    
    if (exitJump != -1) {
        patchJump(exitJump);
        emit(OpCode::POP); // Condition.
    }

    compiler.endScope();
}

void Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    
    int thenJump = emitJump(OpCode::JUMP_IF_FALSE);
    emit(OpCode::POP);
    statement();
    int elseJump = emitJump(OpCode::JUMP);
    
    patchJump(thenJump);
    emit(OpCode::POP);
    if (match(TokenType::ELSE)) { statement(); }
    patchJump(elseJump);
}

void Parser::declaration() {
    if (match(TokenType::VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    
    if (panicMode) synchronize();
}

void Parser::statement() {
    if (match(TokenType::PRINT)) {
        printStatement();
    } else if (match(TokenType::FOR)) {
        forStatement();
    } else if (match(TokenType::IF)) {
        ifStatement();
    } else if (match(TokenType::WHILE)) {
        whileStatement();
    } else if (match(TokenType::LEFT_BRACE)) {
        compiler.beginScope();
        block();
        compiler.endScope();
    } else {
        expressionStatement();
    }
}

void Parser::printStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emit(OpCode::PRINT);
}

void Parser::whileStatement() {
    int loopStart = currentChunk().count();
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    
    int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    
    emit(OpCode::POP);
    statement();
    
    emitLoop(loopStart);
    
    patchJump(exitJump);
    emit(OpCode::POP);
}

void Parser::synchronize() {
    panicMode = false;
    
    while (current.type() != TokenType::_EOF) {
        if (previous.type() == TokenType::SEMICOLON) return;
        
        switch (current.type()) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
                
            default:
                // Do nothing.
                ;
        }
        
        advance();
    }
}

void Parser::errorAt(const Token& token, const std::string& message) {
    if (panicMode) return;
    
    panicMode = true;
    
    std::cerr << "[line " << token.line() << "] Error";
    if (token.type() == TokenType::_EOF) {
        std::cerr << " at end";
    } else if (token.type() == TokenType::ERROR) {
        // Nothing.
    } else {
        std::cerr << " at '" << token.text() << "'";
    }
    
    std::cerr << ": " << message << std::endl;
    hadError = true;
}
