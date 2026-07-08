#pragma once

#include "ast.h"
#include "lexer.h"
#include <string>
#include <stdexcept>

namespace tiny {

// 语法分析错误异常
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

// 语法分析器
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    
    std::unique_ptr<Program> parse();
    
private:
    std::vector<Token> tokens_;
    size_t current_;
    
    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool expect(TokenType type, const std::string& message);
    const Token& consume(TokenType type, const std::string& message);
    
    void synchronize();
    
    // 解析规则
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> multiplication();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();
    
    // 解析语句
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> exprStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> letStatement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<BlockStmt> blockStatement();
    
    // 解析声明
    std::unique_ptr<FunctionDecl> functionDecl();
};

} // namespace tiny
