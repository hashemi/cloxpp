//
//  chunk.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-08-29.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#ifndef chunk_hpp
#define chunk_hpp

#include "common.hpp"
#include "value.hpp"

enum class OpCode: uint8_t {
    CONSTANT,
    NIL,
    TRUE,
    FALSE,
    POP,
    GET_LOCAL,
    GET_GLOBAL,
    DEFINE_GLOBAL,
    SET_LOCAL,
    SET_GLOBAL,
    EQUAL,
    GREATER,
    LESS,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    NOT,
    NEGATE,
    PRINT,
    JUMP,
    JUMP_IF_FALSE,
    LOOP,
    RETURN,
};

class Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

public:
    uint8_t getCode(int offset) const { return code[offset]; };
    void setCode(int offset, uint8_t value) { code[offset] = value; }
    Value getConstant(int constant) const { return constants[constant]; };
    void write(uint8_t byte, int line);
    void write(OpCode opcode, int line);
    unsigned long addConstant(Value value);
    int disassembleInstruction(int offset);
    void disassemble(const std::string& name);
    int getLine(int instruction) { return lines[instruction]; }
    int count() { return static_cast<int>(code.size()); }
};

#endif /* chunk_hpp */
