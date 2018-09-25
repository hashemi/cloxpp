//
//  vm.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "vm.hpp"

InterpretResult VM::interpret(const std::string& source) {
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

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    std::cerr << std::endl;
    
    std::cerr << "[line " << chunk.getLine(ip) << "] in script" << std::endl;
    resetStack();
}

bool VM::binaryOp(std::function<Value(double, double)> op) {
    try {
        auto b = mpark::get<double>(peek(0));
        auto a = mpark::get<double>(peek(1));
        
        popTwoAndPush(op(a, b));
        return true;
    } catch (mpark::bad_variant_access&) {
        runtimeError("Operands must be numbers.");
        return false;
    }
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void VM::popTwoAndPush(Value v) {
    pop();
    pop();
    push(v);
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
        if (!binaryOp([](double a, double b) -> Value { return a op b; })) { \
            return InterpretResult::RUNTIME_ERROR; \
        } \
    } while (false)
        
        auto instruction = OpCode(readByte());
        switch (instruction) {
            case OpCode::CONSTANT: {
                auto constant = readConstant();
                push(constant);
                break;
            }
            case OpCode::NIL:   push(mpark::monostate()); break;
            case OpCode::TRUE:  push(true); break;
            case OpCode::FALSE: push(false); break;
                
            case OpCode::EQUAL: {
                popTwoAndPush(peek(0) == peek(1));
                break;
            }
                
            case OpCode::GREATER:   BINARY_OP(>); break;
            case OpCode::LESS:      BINARY_OP(<); break;
                
            case OpCode::ADD: {
                auto success = mpark::visit(overloaded {
                    [this](double b, double a) -> bool {
                        this->popTwoAndPush(a + b);
                        return true;
                    },
                    [this](std::string b, std::string a) -> bool {
                        this->popTwoAndPush(a + b);
                        return true;
                    },
                    [this](auto a, auto b) -> bool {
                        this->runtimeError("Operands must be two numbers or two strings.");
                        return false;
                    }
                }, peek(0), peek(1));
                
                if (!success)
                    return InterpretResult::RUNTIME_ERROR;
                
                break;
            }
                
            case OpCode::SUBTRACT:  BINARY_OP(-); break;
            case OpCode::MULTIPLY:  BINARY_OP(*); break;
            case OpCode::DIVIDE:    BINARY_OP(/); break;
            case OpCode::NOT: push(isFalsy(pop())); break;
            
            case OpCode::NEGATE:
                try {
                    auto negated = -mpark::get<double>(peek(0));
                    pop(); // if we get here it means it was good
                    push(negated);
                } catch (mpark::bad_variant_access&) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
                
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
