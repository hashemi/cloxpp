//
//  main.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-08-29.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include <fstream>
#include "common.hpp"
#include "chunk.hpp"
#include "vm.hpp"

static void repl(VM& vm) {
    std::string line;
    while (true) {
        std::cout << "> ";
        getline(std::cin, line);
        
        vm.interpret(line);
        
        if (std::cin.eof()) {
            std::cout << std::endl;
            break;
        }
    }
}

static std::string readFile(const std::string path) {
    std::ifstream t(path);
    std::string str;
    
    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    
    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    
    return str;
}

static void runFile(VM& vm, const std::string path) {
    auto source = readFile(path);
    auto result = vm.interpret(source);
    
    switch (result) {
        case InterpretResult::OK: break;
        case InterpretResult::COMPILE_ERROR: exit(65);
        case InterpretResult::RUNTIME_ERROR: exit(70);
    }
}

int main(int argc, const char * argv[]) {
    auto c = Chunk();
    auto vm = VM(c);
    
    if (argc == 1) {
        repl(vm);
    } else if (argc == 2) {
        runFile(vm, argv[1]);
    } else {
        std::cerr << "Usage: cloxpp [path]" << std::endl;
        exit(64);
    }
    
    return 0;
}
