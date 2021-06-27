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

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct CallFrame {
    Function function;
    unsigned ip;
    unsigned long stackOffset;
};

class VM {
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, Value> globals;
    
    inline void resetStack() { stack.clear(); frames.clear(); }
    
    void runtimeError(const char* format, ...);
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
    
public:
    InterpretResult interpret(const std::string& source);
    InterpretResult run();
};

#endif /* vm_hpp */
