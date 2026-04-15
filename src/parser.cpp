#include "parser.h"
#include <sstream>

namespace tiny {

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0) {}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    std::ostringstream oss;
    oss << message << " at token '" << peek().lexeme << "' (line " << peek().line << ")";
    throw ParseError(oss.str());
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    
    while (!isAtEnd()) {
        try {
            auto func = functionDecl();
            program->functions.push_back(std::move(func));
        } catch (const ParseError& e) {
            // 错误恢复：跳同步到下一个函数声明
            synchronize();
        }
    }
    
    return program;
}

void Parser::synchronize() {
    advance(); // 跳过错误的 token
    
    while (!isAtEnd()) {
        // 查找函数声明的起始位置
        if (previous().type == TokenType::Semicolon) return;
        if (check(TokenType::Fn)) return;
        
        advance();
    }
}

std::unique_ptr<FunctionDecl> Parser::functionDecl() {
    consume(TokenType::Fn, "Expected 'fn'");
    const Token& name = consume(TokenType::Identifier, "Expected function name");
    
    consume(TokenType::LeftParen, "Expected '('");
    
    std::vector<std::string> params;
    if (!check(TokenType::RightParen)) {
        do {
            if (params.size() >= 255) {
                throw ParseError("Cannot have more than 255 parameters");
            }
            const Token& paramName = consume(TokenType::Identifier, "Expected parameter name");
            params.push_back(paramName.lexeme);
        } while (match(TokenType::Comma));
    }
    
    consume(TokenType::RightParen, "Expected ')'");
    consume(TokenType::LeftBrace, "Expected '{'");
    
    auto body = blockStatement();
    
    return std::make_unique<FunctionDecl>(name.lexeme, std::move(params), std::move(body));
}

std::unique_ptr<BlockStmt> Parser::blockStatement() {
    auto block = std::make_unique<BlockStmt>();
    
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        block->stmts.push_back(statement());
    }
    
    consume(TokenType::RightBrace, "Expected '}' after block");
    return block;
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::Let)) return letStatement();
    if (match(TokenType::If)) return ifStatement();
    if (match(TokenType::While)) return whileStatement();
    if (match(TokenType::Return)) return returnStatement();
    
    return exprStatement();
}

std::unique_ptr<Stmt> Parser::letStatement() {
    const Token& name = consume(TokenType::Identifier, "Expected variable name");
    consume(TokenType::Equal, "Expected '=' after variable name");
    auto init = expression();
    consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    
    return std::make_unique<VarDeclStmt>(name.lexeme, std::move(init));
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LeftParen, "Expected '(' after 'if'");
    auto condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition");
    
    auto thenBody = statement();
    std::unique_ptr<Stmt> elseBody = nullptr;
    
    if (match(TokenType::Else)) {
        elseBody = statement();
    }
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBody), std::move(elseBody));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LeftParen, "Expected '(' after 'while'");
    auto condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition");
    
    auto body = statement();
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    auto value = expression();
    consume(TokenType::Semicolon, "Expected ';' after return value");
    
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::exprStatement() {
    auto expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = equality();
    
    if (match(TokenType::Equal)) {
        auto value = assignment();
        
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            // 这里我们简单地将赋值转换为函数调用
            // 在实际实现中，你应该创建一个 AssignmentExpr 节点
            throw ParseError("Assignment not supported in this version");
        } else {
            throw ParseError("Invalid assignment target");
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    
    while (match(TokenType::EqualEqual) || match(TokenType::NotEqual)) {
        BinaryOp op = previous().type == TokenType::EqualEqual ? BinaryOp::Equals : BinaryOp::NotEqual;
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = addition();
    
    while (match(TokenType::Less) || match(TokenType::LessEqual) || 
           match(TokenType::Greater) || match(TokenType::GreaterEqual)) {
        BinaryOp op;
        switch (previous().type) {
            case TokenType::Less: op = BinaryOp::LessThan; break;
            case TokenType::Greater: op = BinaryOp::GreaterThan; break;
            default: continue; // 简化版本，只支持 < 和 >
        }
        auto right = addition();
        expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::addition() {
    auto expr = multiplication();
    
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        BinaryOp op = previous().type == TokenType::Plus ? BinaryOp::Add : BinaryOp::Subtract;
        auto right = multiplication();
        expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::multiplication() {
    auto expr = unary();
    
    while (match(TokenType::Star) || match(TokenType::Slash)) {
        BinaryOp op = previous().type == TokenType::Star ? BinaryOp::Multiply : BinaryOp::Divide;
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenType::Minus)) {
        auto right = unary();
        auto zero = std::make_unique<NumberExpr>(0);
        return std::make_unique<BinaryExpr>(BinaryOp::Subtract, std::move(zero), std::move(right));
    }
    
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary();
    
    while (true) {
        if (match(TokenType::LeftParen)) {
            std::vector<std::unique_ptr<Expr>> args;
            
            if (!check(TokenType::RightParen)) {
                do {
                    args.push_back(expression());
                } while (match(TokenType::Comma));
            }
            
            consume(TokenType::RightParen, "Expected ')' after arguments");
            
            if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
                expr = std::make_unique<CallExpr>(var->name, std::move(args));
            } else {
                throw ParseError("Can only call functions and methods");
            }
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::Number)) {
        return std::make_unique<NumberExpr>(std::stod(previous().lexeme));
    }
    
    if (match(TokenType::Identifier)) {
        return std::make_unique<VariableExpr>(previous().lexeme);
    }
    
    if (match(TokenType::LeftParen)) {
        auto expr = expression();
        consume(TokenType::RightParen, "Expected ')' after expression");
        return expr;
    }
    
    throw ParseError(std::string("Expected expression, got: ") + peek().lexeme);
}

} // namespace tiny
