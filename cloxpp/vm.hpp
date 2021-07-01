//
//  vm.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#ifndef vm_hpp
#define vm_hpp

#include "value.hpp"
#include "compiler.hpp"
#include <unordered_map>

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct CallFrame {
    Closure closure;
    unsigned ip;
    unsigned long stackOffset;
};

static Value clockNative(int argCount, std::vector<Value>::iterator args) {
    return (double)clock() / CLOCKS_PER_SEC;
}

class VM {
    // TODO: Switch to a fixed array to prevent pointer invalidation
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, Value> globals;
    UpvalueValue openUpvalues;
    
    inline void resetStack() {
        stack.clear();
        frames.clear();
        stack.reserve(STACK_MAX);
        openUpvalues = nullptr;
    }
    
    void runtimeError(const char* format, ...);
    void defineNative(const std::string& name, NativeFn function);
    template <typename F>
    bool binaryOp(F op);
    void popTwoAndPush(Value v);
    
    inline void push(Value v) { stack.push_back(v); }
    inline Value pop() {
        auto v = stack.back();
        stack.pop_back();
        return v;
    }
    inline Value const& peek(int distance) { return stack[stack.size() - 1 - distance]; }
    bool callValue(const Value& callee, int argCount);
    UpvalueValue captureUpvalue(Value* local);
    void closeUpvalues(Value* last);
    bool call(Closure closure, int argCount);
    
public:
    explicit VM() {
        stack.reserve(STACK_MAX);
        openUpvalues = nullptr;
        defineNative("clock", clockNative);
    }
    InterpretResult interpret(const std::string& source);
    InterpretResult run();
};

#endif /* vm_hpp */
