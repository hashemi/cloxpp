//
//  vm.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#ifndef vm_hpp
#define vm_hpp

#include "chunk.hpp"
#include "compiler.hpp"

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
    Chunk& chunk;
    unsigned ip;
    std::vector<Value> stack;
    
    inline void resetStack() {
        stack.clear();
    }
    
    inline void push(Value v) {
        stack.push_back(v);
    }
    
    inline Value pop() {
        auto v = stack.back();
        stack.pop_back();
        return v;
    }
    
public:
    VM(Chunk& c): chunk(c), ip(0), stack(std::vector<Value>()) {};
    InterpretResult interpret(std::string source);
    InterpretResult interpret(Chunk chunk);
    InterpretResult run();
};

#endif /* vm_hpp */
