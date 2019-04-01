//
//  compiler.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-02.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "compiler.hpp"

Parser::Parser(const std::string& source, Chunk& chunk) :
    previous(Token(TokenType::_EOF, source, 0)),
    current(Token(TokenType::_EOF, source, 0)),
    scanner(Scanner(source)),
    hadError(false), panicMode(false),
    compilingChunk(chunk)
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

void Parser::endCompiler() {
    emitReturn();
    
#ifdef DEBUG_PRINT_CODE
    if (!hadError) {
        currentChunk().disassemble("code");
    }
#endif
}

void Parser::binary() {
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

void Parser::literal() {
    switch (previous.type()) {
        case TokenType::FALSE: emit(OpCode::FALSE); break;
        case TokenType::NIL:   emit(OpCode::NIL); break;
        case TokenType::TRUE:  emit(OpCode::TRUE); break;
        default:
            return; // Unreachable.
    }
}

void Parser::grouping() {
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Parser::number() {
    Value value = std::stod(std::string(previous.text()));
    emitConstant(value);
}

void Parser::string() {
    auto str = previous.text();
    str.remove_prefix(1);
    str.remove_suffix(1);
    emitConstant(std::string(str));
}

void Parser::namedVariable(const Token& token) {
    auto arg = identifierConstant(token);
    emit(OpCode::GET_GLOBAL, arg);
}

void Parser::variable() {
    namedVariable(previous);
}

void Parser::unary() {
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
    auto grouping = [this]() { this->grouping(); };
    auto unary = [this]() { this->unary(); };
    auto binary = [this]() { this->binary(); };
    auto number = [this]() { this->number(); };
    auto string = [this]() { this->string(); };
    auto literal = [this]() { this->literal(); };
    auto variable = [this]() { this->variable(); };
    
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
        { variable,     nullptr,    Precedence::NONE },       // TOKEN_IDENTIFIER
        { string,      nullptr,    Precedence::NONE },       // TOKEN_STRING
        { number,      nullptr,    Precedence::NONE },       // TOKEN_NUMBER
        { nullptr,     nullptr,    Precedence::AND },        // TOKEN_AND
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_CLASS
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_ELSE
        { literal,     nullptr,    Precedence::NONE },       // TOKEN_FALSE
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_FUN
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_FOR
        { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_IF
        { literal,     nullptr,    Precedence::NONE },       // TOKEN_NIL
        { nullptr,     nullptr,    Precedence::OR },         // TOKEN_OR
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
    
    prefixRule();
    
    while (precedence <= getRule(current.type()).precedence) {
        advance();
        auto infixRule = getRule(previous.type()).infix;
        infixRule();
    }
}

uint8_t Parser::identifierConstant(const Token& token) {
    return makeConstant(std::string(token.text()));
}

uint8_t Parser::parseVariable(const std::string& errorMessage) {
    consume(TokenType::IDENTIFIER, errorMessage);
    return identifierConstant(previous);
}

void Parser::defineVariable(uint8_t global) {
    emit(OpCode::DEFINE_GLOBAL, global);
}

void Parser::expression() {
    parsePrecedence(Precedence::ASSIGNMENT);
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
    } else {
        expressionStatement();
    }
}

void Parser::printStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emit(OpCode::PRINT);
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
        std::cerr << token.text();
    }
    
    std::cerr << ": " << message << std::endl;
    hadError = true;
}
