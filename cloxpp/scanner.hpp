//
//  scanner.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-02.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#ifndef scanner_hpp
#define scanner_hpp

#include <string>

enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS,
    SEMICOLON, SLASH, STAR,
    
    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    
    // Literals.
    IDENTIFIER, STRING, NUMBER,
    
    // Keywords.
    AND, CLASS, ELSE, FALSE,
    FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS,
    TRUE, VAR, WHILE,
    
    ERROR,
    _EOF
};

class Token {
    TokenType _type;
    std::string_view _text;
    int _line;
    
public:
    Token(TokenType type, std::string_view text, int line):
        _type(type), _text(text), _line(line) {};
    
    TokenType type() { return _type; };
    std::string_view text() { return _text; };
    int line() { return _line; };
};

class Scanner {
    std::string source;
    int start;
    int current;
    int line;
    
    bool isAtEnd();
    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    
    Token makeToken(TokenType type);
    Token errorToken(std::string message);
    
    void skipWhitespace();
    TokenType checkKeyword(size_t pos, size_t len, std::string rest, TokenType type);
    TokenType identifierType();
    Token identifier();
    Token number();
    Token string();
    
public:
    Scanner(std::string source):
        source(source),
        start(0),
        current(0),
        line(1) {};
    
    Token scanToken();
};

#endif /* scanner_hpp */
