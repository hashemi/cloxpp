//
//  vm.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "vm.hpp"

#define FRAMES_MAX 64

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

bool VM::callValue(const Value& callee, int argCount) {
    return std::visit(overloaded {
        [this, argCount](Closure closure) -> bool {
            return call(closure, argCount);
        },
        [this, argCount](NativeFunction native) -> bool {
            auto result = native->function(argCount, stack.end() - argCount);
            stack.resize(stack.size() - argCount - 1);
            push(result);
            return true;
        },
        [this](auto v) -> bool {
            this->runtimeError("Can only call functions and classes.");
            return false;
        }
    }, callee);
}

bool VM::call(Closure closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.",
                     closure->function->arity, argCount);
        return false;
    }
    
    if (frames.size() + 1 == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    frames.emplace_back(CallFrame());
    auto& frame = frames.back();
    frame.ip = 0;
    frame.closure = closure;
    frame.stackOffset = stack.size() - argCount - 1;
    
    return true;
}

InterpretResult VM::interpret(const std::string& source) {
    auto parser = Parser(source);
    auto opt = parser.compile();
    if (!opt) { return InterpretResult::COMPILE_ERROR; }

    auto& function = *opt;
    auto closure = std::make_shared<ClosureObject>(function);
    push(closure);
    call(closure, 0);

    return run();
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    std::cerr << std::endl;
    
    for (auto i = frames.size(); i-- > 0; ) {
        auto& frame = frames[i];
        auto function = frame.closure->function;
        auto line = function->getChunk().getLine(frame.ip);
        std::cerr << "[line " << line << "] in ";
        if (function->name.empty()) {
            std::cerr << "script" << std::endl;
        } else {
            std::cerr << function->name << "()" << std::endl;
        }
    }

    resetStack();
}

void VM::defineNative(const std::string& name, NativeFn function) {
    auto obj = std::make_shared<NativeFunctionObject>();
    obj->function = function;
    globals[name] = obj;
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

void VM::popTwoAndPush(Value v) {
    pop();
    pop();
    push(v);
}

InterpretResult VM::run() {
    auto readByte = [this]() -> uint8_t {
        return this->frames.back().closure->function->getCode(this->frames.back().ip++);
    };
    
    auto readConstant = [this, readByte]() -> const Value& {
        return this->frames.back().closure->function->getConstant(readByte());
    };
    
    auto readShort = [this]() -> uint16_t {
        this->frames.back().ip += 2;
        return ((this->frames.back().closure->function->getCode(this->frames.back().ip - 2) << 8) | (this->frames.back().closure->function->getCode(this->frames.back().ip - 1)));
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
        
        frames.back().closure->function->getChunk().disassembleInstruction(frames.back().ip);
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
                push(stack[frames.back().stackOffset + slot]);
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
                stack[frames.back().stackOffset + slot] = peek(0);
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
                frames.back().ip += offset;
                break;
            }
                
            case OpCode::LOOP: {
                auto offset = readShort();
                frames.back().ip -= offset;
                break;
            }
                
            case OpCode::JUMP_IF_FALSE: {
                auto offset = readShort();
                if (isFalsy(peek(0))) {
                    frames.back().ip += offset;
                }
                break;
            }
                
            case OpCode::CALL: {
                int argCount = readByte();
                if (!callValue(peek(argCount), argCount)) {
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
                
            case OpCode::CLOSURE: {
                auto function = std::get<Function>(readConstant());
                auto closure = std::make_shared<ClosureObject>(function);
                push(closure);
                break;
            }
            
            case OpCode::RETURN: {
                auto result = pop();
                
                auto lastOffset = frames.back().stackOffset;
                frames.pop_back();
                if (frames.empty()) {
                    pop();
                    return InterpretResult::OK;
                }

                stack.resize(lastOffset);
                push(result);
                break;
            }
        }
    }
    
    return InterpretResult::OK;

#undef BINARY_OP
}
