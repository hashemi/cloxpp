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

class Parser {
    Token current;
    Token previous;
    Scanner scanner;
    
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
    void emitReturn();
    uint8_t makeConstant(Value value);
    void emitConstant(Value value);
    
    void endCompiler();
    
    void binary(bool canAssign);
    void literal(bool canAssign);
    void grouping(bool canAssign);
    void number(bool canAssign);
    void string(bool canAssign);
    void namedVariable(const Token& token, bool canAssign);
    void variable(bool canAssign);
    void unary(bool canAssign);
    ParseRule& getRule(TokenType type);
    void parsePrecedence(Precedence precedence);
    uint8_t identifierConstant(const Token& token);
    uint8_t parseVariable(const std::string& errorMessage);
    void defineVariable(uint8_t global);
    void expression();
    void varDeclaration();
    void expressionStatement();
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
    
    
public:
    Parser(const std::string& source, Chunk& chunk);
    bool compile();
};

#endif /* compiler_hpp */
