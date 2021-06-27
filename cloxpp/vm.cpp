//
//  vm.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "vm.hpp"

InterpretResult VM::interpret(const std::string& source) {
    auto parser = Parser(source);
    auto opt = parser.compile();
    if (!opt) { return InterpretResult::COMPILE_ERROR; }

    auto& function = *opt;
    push(function);
    
    auto frame = CallFrame();
    frame.ip = 0;
    frame.function = function;
    frame.stackOffset = stack.size();
    frames.emplace_back(frame);

    return run();
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    std::cerr << std::endl;
    
    auto& frame = frames.back();
    std::cerr << "[line " << frame.function->getChunk().getLine(frame.ip) << "] in script" << std::endl;
    resetStack();
}

template <typename F>
bool VM::binaryOp(F op) {
    try {
        auto b = std::get<double>(peek(0));
        auto a = std::get<double>(peek(1));
        
        popTwoAndPush(op(a, b));
        return true;
    } catch (std::bad_variant_access&) {
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
    auto& frame = frames.back();
    
    auto readByte = [this, &frame]() -> uint8_t {
        return frame.function->getCode(frame.ip++);
    };
    
    auto readConstant = [this, readByte, &frame]() -> const Value& {
        return frame.function->getConstant(readByte());
    };
    
    auto readShort = [this, &frame]() -> uint16_t {
        frame.ip += 2;
        return ((frame.function->getCode(frame.ip - 2) << 8) | (frame.function->getCode(frame.ip - 1)));
    };
    
    auto readString = [this, readConstant]() -> const std::string& {
        return std::get<std::string>(readConstant());
    };
    
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "          ";
        for (auto value: stack) {
            std::cout << "[ " << value << " ]";
        }
        std::cout << std::endl;
        
        frame.function->getChunk().disassembleInstruction(frame.ip);
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
            case OpCode::NIL:   push(std::monostate()); break;
            case OpCode::TRUE:  push(true); break;
            case OpCode::FALSE: push(false); break;
            case OpCode::POP: pop(); break;
                
            case OpCode::GET_LOCAL: {
                uint8_t slot = readByte();
                push(stack[frame.stackOffset + slot]);
                break;
            }
                
            case OpCode::GET_GLOBAL: {
                auto name = readString();
                auto found = globals.find(name);
                if (found == globals.end()) {
                    runtimeError("Undefined variable '%s'.", name.c_str());
                    return InterpretResult::RUNTIME_ERROR;
                }
                auto value = found->second;
                push(value);
                break;
            }
                
            case OpCode::DEFINE_GLOBAL: {
                auto name = readString();
                globals[name] = peek(0);
                pop();
                break;
            }
                
            case OpCode::SET_LOCAL: {
                uint8_t slot = readByte();
                stack[slot] = peek(0);
                break;
            }
                
            case OpCode::SET_GLOBAL: {
                auto name = readString();
                auto found = globals.find(name);
                if (found == globals.end()) {
                    runtimeError("Undefined variable '%s'.", name.c_str());
                    return InterpretResult::RUNTIME_ERROR;
                }
                found->second = peek(0);
                break;
            }

            case OpCode::EQUAL: {
                popTwoAndPush(peek(0) == peek(1));
                break;
            }
                
            case OpCode::GREATER:   BINARY_OP(>); break;
            case OpCode::LESS:      BINARY_OP(<); break;
                
            case OpCode::ADD: {
                auto success = std::visit(overloaded {
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
                    auto negated = -std::get<double>(peek(0));
                    pop(); // if we get here it means it was good
                    push(negated);
                } catch (std::bad_variant_access&) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
                
            case OpCode::PRINT: {
                std::cout << pop() << std::endl;
                break;
            }
            
            case OpCode::JUMP: {
                auto offset = readShort();
                frame.ip += offset;
                break;
            }
                
            case OpCode::LOOP: {
                auto offset = readShort();
                frame.ip -= offset;
                break;
            }
                
            case OpCode::JUMP_IF_FALSE: {
                auto offset = readShort();
                if (isFalsy(peek(0))) {
                    frame.ip += offset;
                }
                break;
            }
            
            case OpCode::RETURN: {
                // Exit interpreter.
                return InterpretResult::OK;
            }
        }
    }
    
    return InterpretResult::OK;

#undef BINARY_OP
}
