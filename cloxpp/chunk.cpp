//
//  chunk.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-08-29.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "chunk.hpp"

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

void Chunk::disassemble(const std::string& name) {
    std::cout << "== " << name << " ==" << std::endl;
    
    for (int i = 0; i < code.size();) {
        i = disassembleInstruction(i);
    }
}

static int simpleInstruction(const std::string& name, int offset) {
    std::cout << name << std::endl;
    return offset + 1;
}

static int constantInstruction(const std::string& name, const Chunk& chunk, int offset) {
    auto constant = chunk.getCode(offset + 1);
    printf("%-16s %4d '", name.c_str(), constant);
    std::cout << chunk.getConstant(constant);
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
            return constantInstruction("OP_CONSTANT", *this, offset);
        case OpCode::NIL:
            return simpleInstruction("OP_NIL", offset);
        case OpCode::TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OpCode::FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OpCode::POP:
            return simpleInstruction("OP_POP", offset);
        case OpCode::GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", *this, offset);
        case OpCode::DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", *this, offset);
        case OpCode::EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OpCode::GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OpCode::LESS:
            return simpleInstruction("OP_LESS", offset);
        case OpCode::ADD:
            return simpleInstruction("OP_ADD", offset);
        case OpCode::SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OpCode::MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OpCode::DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OpCode::NOT:
            return simpleInstruction("OP_NOT", offset);
        case OpCode::NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OpCode::PRINT:
            return simpleInstruction("OP_PRINT", offset);
        case OpCode::RETURN:
            return simpleInstruction("OP_RETURN", offset);
    }
    
    std::cout << "Unknown opcode: " << code[offset] << std::endl;
    return offset + 1;
}

