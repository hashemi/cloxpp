//
//  main.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-08-29.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "common.hpp"
#include "chunk.hpp"
#include "vm.hpp"

int main(int argc, const char * argv[]) {
    auto c = Chunk();
    
    auto constant = c.addConstant(1.2);
    c.write(OpCode::CONSTANT, 123);
    c.write(constant, 123);
    
    constant = c.addConstant(3.4);
    c.write(OpCode::CONSTANT, 123);
    c.write(constant, 123);
    
    c.write(OpCode::ADD, 123);
    
    constant = c.addConstant(5.6);
    c.write(OpCode::CONSTANT, 123);
    c.write(constant, 123);
    
    c.write(OpCode::DIVIDE, 123);
    c.write(OpCode::NEGATE, 123);
    
    c.write(OpCode::RETURN, 123);
    
    auto vm = VM(c);
    vm.interpret();
    
    return 0;
}
