//
//  chunk.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-08-29.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "chunk.hpp"

Chunk::Chunk():
    code(std::vector<uint8_t>()),
    constants(std::vector<Value>()),
    lines(std::vector<int>())
{ }

void Chunk::write(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
}

void Chunk::write(OpCode opcode, int line) {
    write(static_cast<uint8_t>(opcode), line);
}

unsigned long Chunk::addConstant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
}

void Chunk::disassemble(const std::string name) {
    std::cout << "== " << name << " ==" << std::endl;
    
    for (int i = 0; i < code.size();) {
        i = disassembleInstruction(i);
    }
}

static int simpleInstruction(const std::string name, int offset) {
    std::cout << name << std::endl;
    return offset + 1;
}

static int constantInstruction(const std::string name, const Chunk* chunk, int offset) {
    auto constant = chunk->getCode(offset + 1);
    printf("%-16s %4d '", name.c_str(), constant);
    std::cout << chunk->getConstant(constant);
    printf("'\n");
    return offset + 2;
}

int Chunk::disassembleInstruction(int offset) {
    printf("%04d ", offset);
    
    if (offset > 0 && lines[offset] == lines[offset - 1]) {
        std::cout << "   | ";
    } else {
        printf("%4d ", lines[offset]);
    }
    
    auto instruction = OpCode(code[offset]);
    switch (instruction) {
        case OpCode::CONSTANT:
            return constantInstruction("OP_CONSTANT", this, offset);
        case OpCode::RETURN:
            return simpleInstruction("OP_RETURN", offset);
    }
    
    std::cout << "Unknown opcode: " << code[offset] << std::endl;
    return offset + 1;
}

