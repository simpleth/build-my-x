#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace tiny {

// AST 节点基类
struct ASTNode {
    virtual ~ASTNode() = default;
};

// 表达式节点
struct Expr : ASTNode {};

// 语句节点
struct Stmt : ASTNode {};

// 字面量表达式
struct NumberExpr : Expr {
    double value;
    NumberExpr(double val) : value(val) {}
};

// 变量引用表达式
struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
};

// 二元运算符
enum class BinaryOp {
    Add,       // +
    Subtract,  // -
    Multiply,  // *
    Divide,    // /
    Equals,    // ==
    NotEqual,  // !=
    LessThan,  // <
    GreaterThan // >
};

// 二元表达式
struct BinaryExpr : Expr {
    BinaryOp op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
};

// 函数调用表达式
struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    
    CallExpr(const std::string& c, std::vector<std::unique_ptr<Expr>> a)
        : callee(c), args(std::move(a)) {}
};

// 表达式语句
struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};

// 返回语句
struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
};

// 变量声明语句
struct VarDeclStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> init;
    VarDeclStmt(const std::string& n, std::unique_ptr<Expr> i)
        : name(n), init(std::move(i)) {}
};

// if 语句
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBody;
    std::unique_ptr<Stmt> elseBody; // 可以为空
    
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> then, std::unique_ptr<Stmt> els = nullptr)
        : condition(std::move(cond)), thenBody(std::move(then)), elseBody(std::move(els)) {}
};

// while 循环语句
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// 块语句（多条语句）
struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    BlockStmt() = default;
};

// 函数声明
struct FunctionDecl {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<BlockStmt> body;
    
    FunctionDecl(const std::string& n, std::vector<std::string> p, std::unique_ptr<BlockStmt> b)
        : name(n), params(std::move(p)), body(std::move(b)) {}
};

// 程序（顶层结构）
struct Program {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
};

} // namespace tiny
