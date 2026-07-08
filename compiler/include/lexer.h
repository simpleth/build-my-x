#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace tiny {

// Token 类型枚举
enum class TokenType {
    // 关键字
    Fn, Let, If, Else, While, Return,
    
    // 标识符和字面量
    Identifier, Number,
    
    // 运算符
    Plus, Minus, Star, Slash, Equal, EqualEqual,
    Less, LessEqual, Greater, GreaterEqual, NotEqual,
    
    // 分隔符
    LeftParen, RightParen, LeftBrace, RightBrace,
    Comma, Semicolon,
    
    // 特殊
    Eof, Error
};

// Token 结构
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    
    Token(TokenType t, const std::string& l, int ln)
        : type(t), lexeme(l), line(ln) {}
};

// 词法分析器
class Lexer {
public:
    Lexer(std::string_view source);
    
    std::vector<Token> tokenize();
    
private:
    std::string_view source_;
    size_t start_;
    size_t current_;
    int line_;
    
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    
    void skipWhitespace();
    void skipLineComment();
    
    Token makeToken(TokenType type);
    Token errorToken(const std::string& message);
    
    Token identifierOrKeyword();
    
    TokenType checkKeyword(int start, int length, const std::string& rest, TokenType yes, TokenType no) const;
    TokenType identifierType() const;
    
    Token number();
    Token string();
};

} // namespace tiny
