#include "lexer.h"
#include <cctype>
#include <unordered_map>

namespace tiny {

Lexer::Lexer(std::string_view source)
    : source_(source), start_(0), current_(0), line_(1) {}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

char Lexer::advance() {
    current_++;
    return source_[current_ - 1];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    return true;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line_++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    // 行注释：跳过到行尾
                    skipLineComment();
                } else {
                    // 除法运算符，不跳过，交给主循环处理
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void Lexer::skipLineComment() {
    while (peek() != '\n' && !isAtEnd()) {
        advance();
    }
    if (!isAtEnd()) {
        line_++;
        advance();
    }
}

Token Lexer::makeToken(TokenType type) {
    return Token(type, std::string(source_.substr(start_, current_ - start_)), line_);
}

Token Lexer::errorToken(const std::string& message) {
    return Token(TokenType::Error, message, line_);
}

Token Lexer::number() {
    while (std::isdigit(peek())) {
        advance();
    }
    
    // 支持小数
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance(); // 跳过 '.'
        while (std::isdigit(peek())) {
            advance();
        }
    }
    
    return makeToken(TokenType::Number);
}

Token Lexer::identifierOrKeyword() {
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    
    return makeToken(identifierType());
}

TokenType Lexer::checkKeyword(int start, int length, const std::string& rest, TokenType yes, TokenType no) const {
    // current_ - start_ 是整个标识符的长度
    // start 是已匹配的起始位置，length 是剩余需要匹配的字符数
    if (current_ - start_ == start + length && 
        source_.substr(start_ + start, length) == rest) {
        return yes;
    }
    return no;
}

TokenType Lexer::identifierType() const {
    // 检查关键字
    switch (source_[start_]) {
        case 'f':
            return checkKeyword(1, 1, "n", TokenType::Fn, TokenType::Identifier);
        case 'l':
            return checkKeyword(1, 2, "t", TokenType::Let, TokenType::Identifier);
        case 'i':
            return checkKeyword(1, 1, "f", TokenType::If, TokenType::Identifier);
        case 'e':
            return checkKeyword(1, 3, "se", TokenType::Else, TokenType::Identifier);
        case 'w':
            return checkKeyword(1, 4, "hile", TokenType::While, TokenType::Identifier);
        case 'r':
            return checkKeyword(1, 5, "turn", TokenType::Return, TokenType::Identifier);
    }
    
    return TokenType::Identifier;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        start_ = current_;
        skipWhitespace();
        
        if (isAtEnd()) break;
        
        // 在跳过空白和注释后更新 start 位置
        start_ = current_;
        
        char c = advance();
        
        if (std::isdigit(c)) {
            tokens.push_back(number());
        } else if (std::isalpha(c) || c == '_') {
            tokens.push_back(identifierOrKeyword());
        } else {
            switch (c) {
                case '(': tokens.push_back(makeToken(TokenType::LeftParen)); break;
                case ')': tokens.push_back(makeToken(TokenType::RightParen)); break;
                case '{': tokens.push_back(makeToken(TokenType::LeftBrace)); break;
                case '}': tokens.push_back(makeToken(TokenType::RightBrace)); break;
                case ',': tokens.push_back(makeToken(TokenType::Comma)); break;
                case ';': tokens.push_back(makeToken(TokenType::Semicolon)); break;
                case '+': tokens.push_back(makeToken(TokenType::Plus)); break;
                case '-': tokens.push_back(makeToken(TokenType::Minus)); break;
                case '*': tokens.push_back(makeToken(TokenType::Star)); break;
                case '/': tokens.push_back(makeToken(TokenType::Slash)); break;
                case '=':
                    tokens.push_back(match('=') ? makeToken(TokenType::EqualEqual) : makeToken(TokenType::Equal));
                    break;
                case '<':
                    tokens.push_back(match('=') ? makeToken(TokenType::LessEqual) : makeToken(TokenType::Less));
                    break;
                case '>':
                    tokens.push_back(match('=') ? makeToken(TokenType::GreaterEqual) : makeToken(TokenType::Greater));
                    break;
                case '!':
                    tokens.push_back(match('=') ? makeToken(TokenType::NotEqual) : errorToken("Expected '='"));
                    break;
                default:
                    tokens.push_back(errorToken(std::string("Unexpected character: ") + c));
                    break;
            }
        }
    }
    
    tokens.push_back(makeToken(TokenType::Eof));
    return tokens;
}

} // namespace tiny
