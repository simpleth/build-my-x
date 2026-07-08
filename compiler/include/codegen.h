#pragma once

#include "ast.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <map>
#include <string>

namespace tiny {

// 代码生成器
class CodeGen {
public:
    CodeGen();
    
    // 编译整个程序
    bool compile(const Program& program, const std::string& outputFile);
    
    // 输出 LLVM IR 到字符串
    std::string dumpIR() const;
    
private:
    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;
    
    // 符号表：变量名 -> LLVM Value
    std::map<std::string, llvm::Value*> namedValues_;
    
    // 当前函数
    llvm::Function* currentFunction_;
    
    // 生成函数声明
    llvm::Function* createFunctionDecl(const FunctionDecl& func);
    
    // 生成函数定义
    void generateFunction(const FunctionDecl& func);
    
    // 生成语句
    void generateStatement(const Stmt& stmt);
    
    // 生成表达式，返回 LLVM Value
    llvm::Value* generateExpression(const Expr& expr);
    
    // 辅助函数：声明外部 print 函数
    void declarePrintFunction();
    
    // 辅助函数：创建主模块
    void initializeModule();
};

} // namespace tiny
