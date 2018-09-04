//
//  vm.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "vm.hpp"

InterpretResult VM::interpret(Chunk newChunk) {
    chunk = newChunk;
    ip = 0;
    return run();
}

InterpretResult VM::interpret(std::string source) {
    // create a new chunk
    auto newChunk = Chunk();
    auto parser = Parser(source, newChunk);
    
    if (!parser.compile()) {
        return InterpretResult::COMPILE_ERROR;
    }
    
    chunk = newChunk;
    ip = 0;
    
    return run();
}

InterpretResult VM::run() {
    auto readByte = [this]() -> uint8_t {
        return this->chunk.getCode(this->ip++);
    };
    
    auto readConstant = [this, readByte]() -> Value {
        return this->chunk.getConstant(readByte());
    };
    
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "          ";
        for (auto value: stack) {
            std::cout << "[ " << value << " ]";
        }
        std::cout << std::endl;
        
        chunk.disassembleInstruction(ip);
#endif

#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)
        
        auto instruction = OpCode(readByte());
        switch (instruction) {
            case OpCode::CONSTANT: {
                auto constant = readConstant();
                push(constant);
                break;
            }
                
            case OpCode::ADD:       BINARY_OP(+); break;
            case OpCode::SUBTRACT:  BINARY_OP(-); break;
            case OpCode::MULTIPLY:  BINARY_OP(*); break;
            case OpCode::DIVIDE:    BINARY_OP(/); break;
                
            case OpCode::NEGATE: push(-pop()); break;
                
            case OpCode::RETURN: {
                std::cout << pop() << std::endl;
                return InterpretResult::OK;
                break;
            }
        }
    }
    
    return InterpretResult::OK;

#undef BINARY_OP
}
