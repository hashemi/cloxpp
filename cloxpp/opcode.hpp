//
//  opcode.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2021-06-27.
//  Copyright © 2021 Ahmad Alhashemi. All rights reserved.
//

#ifndef opcode_hpp
#define opcode_hpp

#include "common.hpp"

enum class OpCode: uint8_t {
    CONSTANT,
    NIL,
    TRUE,
    FALSE,
    POP,
    GET_LOCAL,
    GET_GLOBAL,
    DEFINE_GLOBAL,
    SET_LOCAL,
    SET_GLOBAL,
    GET_UPVALUE,
    SET_UPVALUE,
    EQUAL,
    GREATER,
    LESS,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    NOT,
    NEGATE,
    PRINT,
    JUMP,
    JUMP_IF_FALSE,
    LOOP,
    CALL,
    CLOSURE,
    RETURN,
};


#endif /* opcode_hpp */
