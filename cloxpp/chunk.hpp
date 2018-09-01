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
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    NEGATE,
    RETURN,
};

class Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

public:
    Chunk();
    uint8_t getCode(int offset) const { return code[offset]; };
    Value getConstant(int constant) const { return constants[constant]; };
    void write(uint8_t byte, int line);
    void write(OpCode opcode, int line);
    unsigned long addConstant(Value value);
    int disassembleInstruction(int offset);
    void disassemble(const std::string name);
};

#endif /* chunk_hpp */
