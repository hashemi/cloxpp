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

typedef void (Parser::*ParseFn)();

struct ParseRule {
    std::function<void()> prefix;
    std::function<void()> infix;
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
    
    void emit(uint8_t byte);
    void emit(OpCode op);
    void emit(OpCode op, uint8_t byte);
    void emit(OpCode op1, OpCode op2);
    void emitReturn();
    uint8_t makeConstant(Value value);
    void emitConstant(Value value);
    
    void endCompiler();
    
    void binary();
    void literal();
    void grouping();
    void number();
    void unary();
    ParseRule& getRule(TokenType type);
    void parsePrecedence(Precedence precedence);
    void expression();
    
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
