//
//  compiler.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-02.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#ifndef compiler_hpp
#define compiler_hpp

#include "scanner.hpp"
#include "chunk.hpp"
#include <iostream>

enum class Precedence {
    NONE,
    ASSIGNMENT,  // =
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    TERM,        // + -
    FACTOR,      // * /
    UNARY,       // ! - +
    CALL,        // . () []
    PRIMARY
};

class Parser;

typedef void (Parser::*ParseFn)(bool canAssign);

struct ParseRule {
    std::function<void(bool)> prefix;
    std::function<void(bool)> infix;
    Precedence precedence;
};

struct Local {
    std::string name;
    int depth;
    Local(std::string name, int depth): name(name), depth(depth) {};
};

class Parser;

class Compiler {
    std::vector<Local> locals;
    int scopeDepth = 0;
    Parser* parser;

public:
    explicit Compiler(Parser* parser): parser(parser) {};
    void addLocal(const std::string& name);
    void declareVariable(const std::string& name);
    void markInitialized();
    int resolveLocal(const std::string& name);
    void beginScope();
    void endScope();
    bool isLocal();
};

class Parser {
    Token current;
    Token previous;
    Scanner scanner;
    Compiler compiler;
    
    bool hadError;
    bool panicMode;
    
    Chunk& compilingChunk;
    
    void advance();
    void consume(TokenType type, const std::string& message);
    bool check(TokenType type);
    bool match(TokenType type);
    
    void emit(uint8_t byte);
    void emit(OpCode op);
    void emit(OpCode op, uint8_t byte);
    void emit(OpCode op1, OpCode op2);
    int emitJump(OpCode op);
    void emitReturn();
    uint8_t makeConstant(Value value);
    void emitConstant(Value value);
    void patchJump(int offset);
    
    void endCompiler();
    
    void binary(bool canAssign);
    void literal(bool canAssign);
    void grouping(bool canAssign);
    void number(bool canAssign);
    void or_(bool canAssign);
    void string(bool canAssign);
    void namedVariable(const std::string& name, bool canAssign);
    void variable(bool canAssign);
    void and_(bool canAssign);
    void unary(bool canAssign);
    ParseRule& getRule(TokenType type);
    void parsePrecedence(Precedence precedence);
    int identifierConstant(const std::string& name);
    uint8_t parseVariable(const std::string& errorMessage);
    void defineVariable(uint8_t global);
    void expression();
    void block();
    void varDeclaration();
    void expressionStatement();
    void ifStatement();
    void declaration();
    void statement();
    void printStatement();
    void synchronize();

    Chunk& currentChunk() {
        return compilingChunk;
    }
    
    void errorAt(const Token& token, const std::string& message);
    
    void error(const std::string& message) {
        errorAt(previous, message);
    };
    
    void errorAtCurrent(const std::string& message) {
        errorAt(current, message);
    };
    
    friend Compiler;
    
public:
    Parser(const std::string& source, Chunk& chunk);
    bool compile();
};

#endif /* compiler_hpp */
