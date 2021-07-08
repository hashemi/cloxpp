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
#include "value.hpp"
#include <iostream>
#include <memory>
#include <optional>

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
    bool isCaptured;
    Local(std::string name, int depth): name(name), depth(depth), isCaptured(false) {};
};

class Upvalue {
public:
    uint8_t index;
    bool isLocal;
    explicit Upvalue(uint8_t index, bool isLocal)
        : index(index), isLocal(isLocal) {}
};

class Parser;

typedef enum {
    TYPE_FUNCTION, TYPE_INITIALIZER, TYPE_METHOD, TYPE_SCRIPT
} FunctionType;

class Compiler {
    Parser* parser;

    FunctionType type;
    Function function;

    std::unique_ptr<Compiler> enclosing;
    
    std::vector<Local> locals;
    std::vector<Upvalue> upvalues;
    int scopeDepth = 0;

public:
    explicit Compiler(Parser* parser, FunctionType type, std::unique_ptr<Compiler> enclosing);
    void addLocal(const std::string& name);
    void declareVariable(const std::string& name);
    void markInitialized();
    int resolveLocal(const std::string& name);
    int resolveUpvalue(const std::string& name);
    int addUpvalue(uint8_t index, bool isLocal);
    void beginScope();
    void endScope();
    bool isLocal();

    friend Parser;
};

class ClassCompiler {
    std::unique_ptr<ClassCompiler> enclosing;
    bool hasSuperclass;
public:
    explicit ClassCompiler(std::unique_ptr<ClassCompiler> enclosing);
    friend Parser;
};

class Parser {
    Token previous;
    Token current;
    Scanner scanner;
    std::unique_ptr<Compiler> compiler;
    std::unique_ptr<ClassCompiler> classCompiler;
    
    bool hadError;
    bool panicMode;
    
    void advance();
    void consume(TokenType type, const std::string& message);
    bool check(TokenType type);
    bool match(TokenType type);
    
    void emit(uint8_t byte);
    void emit(OpCode op);
    void emit(OpCode op, uint8_t byte);
    void emit(OpCode op1, OpCode op2);
    void emitLoop(int loopStart);
    int emitJump(OpCode op);
    void emitReturn();
    uint8_t makeConstant(Value value);
    void emitConstant(Value value);
    void patchJump(int offset);
    
    Function endCompiler();
    
    void binary(bool canAssign);
    void call(bool canAssign);
    void dot(bool canAssign);
    void literal(bool canAssign);
    void grouping(bool canAssign);
    void number(bool canAssign);
    void or_(bool canAssign);
    void string(bool canAssign);
    void namedVariable(const std::string& name, bool canAssign);
    void variable(bool canAssign);
    void super_(bool canAssign);
    void this_(bool canAssign);
    void and_(bool canAssign);
    void unary(bool canAssign);
    ParseRule& getRule(TokenType type);
    void parsePrecedence(Precedence precedence);
    int identifierConstant(const std::string& name);
    uint8_t parseVariable(const std::string& errorMessage);
    void defineVariable(uint8_t global);
    uint8_t argumentList();
    void expression();
    void block();
    void function(FunctionType type);
    void method();
    void classDeclaration();
    void funDeclaration();
    void varDeclaration();
    void expressionStatement();
    void forStatement();
    void ifStatement();
    void declaration();
    void statement();
    void printStatement();
    void returnStatement();
    void whileStatement();
    void synchronize();

    void errorAt(const Token& token, const std::string& message);
    
    void error(const std::string& message) {
        errorAt(previous, message);
    };
    
    void errorAtCurrent(const std::string& message) {
        errorAt(current, message);
    };
    
    friend Compiler;
    
public:
    Parser(const std::string& source);
    Chunk& currentChunk() { return compiler->function->getChunk(); }
    std::optional<Function> compile();
};

#endif /* compiler_hpp */
