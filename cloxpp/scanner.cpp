//
//  scanner.cpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-02.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#include "scanner.hpp"

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

Token Scanner::scanToken() {
    skipWhitespace();
    
    start = current;
    
    if (isAtEnd()) return makeToken(TokenType::_EOF);
    
    auto c = advance();
    
    if (isDigit(c)) return number();
    if (isAlpha(c)) return identifier();
    
    switch (c) {
        case '(': return makeToken(TokenType::LEFT_PAREN);
        case ')': return makeToken(TokenType::RIGHT_PAREN);
        case '{': return makeToken(TokenType::LEFT_BRACE);
        case '}': return makeToken(TokenType::RIGHT_BRACE);
        case ';': return makeToken(TokenType::SEMICOLON);
        case ',': return makeToken(TokenType::COMMA);
        case '.': return makeToken(TokenType::DOT);
        case '-': return makeToken(TokenType::MINUS);
        case '+': return makeToken(TokenType::PLUS);
        case '/': return makeToken(TokenType::SLASH);
        case '*': return makeToken(TokenType::STAR);
        case '!':
            return makeToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        case '=':
            return makeToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        case '<':
            return makeToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        case '>':
            return makeToken(match('=') ?
                             TokenType::GREATER_EQUAL : TokenType::GREATER);
            
        case '"': return string();
    }
    
    return errorToken("Unexpected character.");
}

bool Scanner::isAtEnd() {
    return current == source.length();
}

char Scanner::advance() {
    current++;
    return source[current - 1];
}

char Scanner::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Scanner::peekNext() {
    if ((current + 1) >= source.length()) return '\0';
    return source[current + 1];
}

bool Scanner::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    
    current++;
    return true;
}

Token Scanner::makeToken(TokenType type) {
    auto text = std::string_view(&source[start], current - start);
    return Token(type, text, line);
}

Token Scanner::errorToken(const std::string& message) {
    return Token(TokenType::ERROR, message, line);
}

void Scanner::skipWhitespace() {
    while (true) {
        auto c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
                
            case '\n':
                line++;
                advance();
                break;
                
            case '/':
                if (peekNext() == '/') {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
                
            default:
                return;
        }
    }
}

TokenType Scanner::checkKeyword(size_t pos, size_t len, std::string rest, TokenType type) {
    if (current - start == pos + len && source.compare(start + pos, len, rest) == 0) {
        return type;
    }
    
    return TokenType::IDENTIFIER;
}

TokenType Scanner::identifierType() {
    switch (source[start]) {
        case 'a': return checkKeyword(1, 2, "nd", TokenType::AND);
        case 'c': return checkKeyword(1, 4, "lass", TokenType::CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TokenType::ELSE);
        case 'f':
            if (current - start > 1) {
                switch (source[start + 1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TokenType::FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TokenType::FOR);
                    case 'u': return checkKeyword(2, 1, "n", TokenType::FUN);
                }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TokenType::IF);
        case 'n': return checkKeyword(1, 2, "il", TokenType::NIL);
        case 'o': return checkKeyword(1, 1, "r", TokenType::OR);
        case 'p': return checkKeyword(1, 4, "rint", TokenType::PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TokenType::RETURN);
        case 's': return checkKeyword(1, 4, "uper", TokenType::SUPER);
        case 't':
            if (current - start > 1) {
                switch (source[start + 1]) {
                    case 'h': return checkKeyword(2, 2, "is", TokenType::THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TokenType::TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TokenType::VAR);
        case 'w': return checkKeyword(1, 4, "hile", TokenType::WHILE);
    }
    
    return TokenType::IDENTIFIER;
}

Token Scanner::identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    
    return makeToken(identifierType());
}

Token Scanner::number() {
    while (isDigit(peek())) advance();
    
    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();
        
        while (isDigit(peek())) advance();
    }
    
    return makeToken(TokenType::NUMBER);
}

Token Scanner::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }
    
    if (isAtEnd()) return errorToken("Unterminated string.");
    
    // The closing ".
    advance();
    return makeToken(TokenType::STRING);
}
