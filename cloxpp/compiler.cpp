//
//  compiler.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-02.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "compiler.hpp"

void compile(const std::string source) {
    auto scanner = Scanner(source);
    
    auto line = -1;
    while (true) {
        auto token = scanner.scanToken();
        if (token.line() != line) {
            printf("%4d ", token.line());
            line = token.line();
        } else {
            std::cout << "   | ";
        }
        printf("%2d ", token.type());
        std::cout << "'" << token.text() << "'" << std::endl;
        
        if (token.type() == TokenType::_EOF) break;
    }
}
