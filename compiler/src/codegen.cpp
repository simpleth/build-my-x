#include "codegen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <sstream>
#include <iostream>

namespace tiny {

CodeGen::CodeGen() 
    : builder_(context_) {
    initializeModule();
}

void CodeGen::initializeModule() {
    module_ = std::make_unique<llvm::Module>("tiny_module", context_);
    declarePrintFunction();
}

void CodeGen::declarePrintFunction() {
    // Declare external function: void @print(i64)
    llvm::FunctionType* printType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context_),
        {llvm::Type::getInt64Ty(context_)},
        false
    );
    
    llvm::Function::Create(
        printType,
        llvm::Function::ExternalLinkage,
        "print",
        module_.get()
    );
}

bool CodeGen::compile(const Program& program, const std::string& outputFile) {
    try {
        // Generate all functions
        for (const auto& func : program.functions) {
            generateFunction(*func);
        }
        
        // Verify the generated IR
        std::string error;
        llvm::raw_string_ostream errorStream(error);
        if (llvm::verifyModule(*module_, &errorStream)) {
            std::cerr << "Module verification failed: " << error << std::endl;
            return false;
        }
        
        // Output IR file
        std::string irFile = outputFile + ".ll";
        std::error_code EC;
        llvm::raw_fd_ostream irStream(irFile, EC);
        if (EC) {
            std::cerr << "Could not open file: " << EC.message() << std::endl;
            return false;
        }
        module_->print(irStream, nullptr);
        irStream.close();
        
        std::cout << "LLVM IR written to: " << irFile << std::endl;
        
        // Generate native object file
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
        
        auto targetTripleStr = llvm::sys::getDefaultTargetTriple();
        llvm::Triple targetTriple(targetTripleStr);
        std::cout << "Target triple: " << targetTriple.str() << std::endl;
        
        module_->setTargetTriple(targetTriple);
        
        std::string errorStr;
        auto target = llvm::TargetRegistry::lookupTarget(targetTriple, errorStr);
        if (!target) {
            std::cerr << "Target lookup failed: " << errorStr << std::endl;
            return false;
        }
        
        std::cout << "Target name: " << target->getName() << std::endl;
        
        auto CPU = "generic";
        auto features = "";
        llvm::TargetOptions opt;
        auto RM = std::optional<llvm::Reloc::Model>();
        auto targetMachine = target->createTargetMachine(
            targetTriple, CPU, features, opt, RM);
        
        if (!targetMachine) {
            std::cerr << "Failed to create target machine" << std::endl;
            return false;
        }
        
        module_->setDataLayout(targetMachine->createDataLayout());
        
        llvm::legacy::PassManager passManager;
        auto fileType = llvm::CodeGenFileType::ObjectFile;
        
        std::string objFile = outputFile + ".o";
        std::error_code EC2;
        llvm::raw_fd_ostream objStream(objFile, EC2);
        if (EC2) {
            std::cerr << "Could not open object file: " << EC2.message() << std::endl;
            return false;
        }
        
        if (targetMachine->addPassesToEmitFile(passManager, objStream, nullptr, fileType)) {
            std::cerr << "TargetMachine can't emit object file for this target" << std::endl;
            std::cerr << "Trying with PIC relocation model..." << std::endl;
            
            // Try with PIC relocation model
            auto RM_PIC = llvm::Reloc::PIC_;
            auto targetMachine2 = target->createTargetMachine(
                targetTriple, CPU, features, opt, RM_PIC);
            
            if (!targetMachine2) {
                std::cerr << "Failed to create target machine with PIC" << std::endl;
                return false;
            }
            
            module_->setDataLayout(targetMachine2->createDataLayout());
            
            if (targetMachine2->addPassesToEmitFile(passManager, objStream, nullptr, fileType)) {
                std::cerr << "Still cannot emit object file" << std::endl;
                return false;
            }
        }
        
        passManager.run(*module_);
        objStream.close();
        
        std::cout << "Object file written to: " << objFile << std::endl;
        std::cout << "Compilation successful!" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Compilation error: " << e.what() << std::endl;
        return false;
    }
}

std::string CodeGen::dumpIR() const {
    std::string str;
    llvm::raw_string_ostream os(str);
    module_->print(os, nullptr);
    return os.str();
}

llvm::Function* CodeGen::createFunctionDecl(const FunctionDecl& func) {
    std::vector<llvm::Type*> paramTypes(func.params.size(), 
        llvm::Type::getInt64Ty(context_));
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt64Ty(context_),
        paramTypes,
        false
    );
    
    return llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        func.name,
        module_.get()
    );
}

void CodeGen::generateFunction(const FunctionDecl& func) {
    // Create function
    currentFunction_ = createFunctionDecl(func);
    
    // Create entry basic block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context_, "entry", currentFunction_);
    builder_.SetInsertPoint(entry);
    
    // Add parameters to symbol table
    namedValues_.clear();
    unsigned idx = 0;
    for (auto& arg : currentFunction_->args()) {
        arg.setName(func.params[idx]);
        namedValues_[func.params[idx]] = &arg;
        ++idx;
    }
    
    // Generate function body
    for (const auto& stmt : func.body->stmts) {
        generateStatement(*stmt);
    }
    
    // If no return statement, default return 0
    if (!currentFunction_->back().getTerminator()) {
        builder_.CreateRet(llvm::ConstantInt::get(context_, llvm::APInt(64, 0)));
    }
}

void CodeGen::generateStatement(const Stmt& stmt) {
    // Check if already has terminator (e.g., return)
    if (currentFunction_->back().getTerminator()) {
        return;
    }
    
    if (auto* exprStmt = dynamic_cast<const ExprStmt*>(&stmt)) {
        generateExpression(*exprStmt->expr);
    } 
    else if (auto* returnStmt = dynamic_cast<const ReturnStmt*>(&stmt)) {
        llvm::Value* retValue = generateExpression(*returnStmt->value);
        builder_.CreateRet(retValue);
    }
    else if (auto* varDecl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        llvm::Value* value = generateExpression(*varDecl->init);
        namedValues_[varDecl->name] = value;
    }
    else if (auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        llvm::Value* cond = generateExpression(*ifStmt->condition);
        cond = builder_.CreateICmpNE(cond, 
            llvm::ConstantInt::get(context_, llvm::APInt(64, 0)), "ifcond");
        
        llvm::Function* func = builder_.GetInsertBlock()->getParent();
        llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context_, "then", func);
        llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context_, "else");
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context_, "ifcont");
        
        builder_.CreateCondBr(cond, thenBB, elseBB);
        
        // Generate then branch
        builder_.SetInsertPoint(thenBB);
        generateStatement(*ifStmt->thenBody);
        if (!builder_.GetInsertBlock()->getTerminator()) {
            builder_.CreateBr(mergeBB);
        }
        
        // Generate else branch
        func->insert(func->end(), elseBB);
        builder_.SetInsertPoint(elseBB);
        if (ifStmt->elseBody) {
            generateStatement(*ifStmt->elseBody);
        }
        if (!builder_.GetInsertBlock()->getTerminator()) {
            builder_.CreateBr(mergeBB);
        }
        
        // Merge
        func->insert(func->end(), mergeBB);
        builder_.SetInsertPoint(mergeBB);
    }
    else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        llvm::Function* func = builder_.GetInsertBlock()->getParent();
        llvm::BasicBlock* condBB = llvm::BasicBlock::Create(context_, "while_cond", func);
        llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(context_, "while_body");
        llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(context_, "while_after");
        
        builder_.CreateBr(condBB);
        builder_.SetInsertPoint(condBB);
        
        llvm::Value* cond = generateExpression(*whileStmt->condition);
        cond = builder_.CreateICmpNE(cond,
            llvm::ConstantInt::get(context_, llvm::APInt(64, 0)), "loopcond");
        builder_.CreateCondBr(cond, bodyBB, afterBB);
        
        func->insert(func->end(), bodyBB);
        builder_.SetInsertPoint(bodyBB);
        generateStatement(*whileStmt->body);
        if (!builder_.GetInsertBlock()->getTerminator()) {
            builder_.CreateBr(condBB);
        }
        
        func->insert(func->end(), afterBB);
        builder_.SetInsertPoint(afterBB);
    }
    else if (auto* blockStmt = dynamic_cast<const BlockStmt*>(&stmt)) {
        for (const auto& s : blockStmt->stmts) {
            generateStatement(*s);
            if (currentFunction_->back().getTerminator()) break;
        }
    }
}

llvm::Value* CodeGen::generateExpression(const Expr& expr) {
    if (auto* numExpr = dynamic_cast<const NumberExpr*>(&expr)) {
        return llvm::ConstantInt::get(context_, llvm::APInt(64, (int64_t)numExpr->value));
    }
    
    if (auto* varExpr = dynamic_cast<const VariableExpr*>(&expr)) {
        llvm::Value* val = namedValues_[varExpr->name];
        if (!val) {
            std::cerr << "Unknown variable: " << varExpr->name << std::endl;
            return llvm::ConstantInt::get(context_, llvm::APInt(64, 0));
        }
        return val;
    }
    
    if (auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
        llvm::Value* left = generateExpression(*binaryExpr->left);
        llvm::Value* right = generateExpression(*binaryExpr->right);
        
        switch (binaryExpr->op) {
            case BinaryOp::Add:
                return builder_.CreateAdd(left, right, "addtmp");
            case BinaryOp::Subtract:
                return builder_.CreateSub(left, right, "subtmp");
            case BinaryOp::Multiply:
                return builder_.CreateMul(left, right, "multmp");
            case BinaryOp::Divide:
                return builder_.CreateSDiv(left, right, "divtmp");
            case BinaryOp::LessThan:
                return builder_.CreateICmpSLT(left, right, "cmptmp");
            case BinaryOp::GreaterThan:
                return builder_.CreateICmpSGT(left, right, "cmptmp");
            case BinaryOp::Equals:
                return builder_.CreateICmpEQ(left, right, "cmptmp");
            case BinaryOp::NotEqual:
                return builder_.CreateICmpNE(left, right, "cmptmp");
            default:
                std::cerr << "Unsupported binary operator" << std::endl;
                return llvm::ConstantInt::get(context_, llvm::APInt(64, 0));
        }
    }
    
    if (auto* callExpr = dynamic_cast<const CallExpr*>(&expr)) {
        llvm::Function* calleeFunc = module_->getFunction(callExpr->callee);
        if (!calleeFunc) {
            std::cerr << "Unknown function: " << callExpr->callee << std::endl;
            return llvm::ConstantInt::get(context_, llvm::APInt(64, 0));
        }
        
        std::vector<llvm::Value*> args;
        for (const auto& arg : callExpr->args) {
            args.push_back(generateExpression(*arg));
        }
        
        // Only add name if the call returns a non-void value
        if (calleeFunc->getReturnType()->isVoidTy()) {
            return builder_.CreateCall(calleeFunc, args);
        } else {
            return builder_.CreateCall(calleeFunc, args, "calltmp");
        }
    }
    
    std::cerr << "Unknown expression type" << std::endl;
    return llvm::ConstantInt::get(context_, llvm::APInt(64, 0));
}

} // namespace tiny
