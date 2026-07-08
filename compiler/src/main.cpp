#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// 读取文件内容
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 打印帮助信息
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] <input.tiny>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -o <output>  Specify output file name (without extension)" << std::endl;
    std::cout << "  --ir         Only output LLVM IR (.ll file)" << std::endl;
    std::cout << "  --help       Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string inputFile;
    std::string outputName;
    bool onlyIR = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                outputName = argv[++i];
            } else {
                std::cerr << "Error: -o requires an argument" << std::endl;
                return 1;
            }
        } else if (arg == "--ir") {
            onlyIR = true;
        } else {
            inputFile = arg;
        }
    }
    
    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // 读取源文件
    std::string source = readFile(inputFile);
    if (source.empty()) {
        return 1;
    }
    
    std::cout << "Compiling: " << inputFile << std::endl;
    
    try {
        // 1. 词法分析
        std::cout << "Phase 1: Lexical Analysis..." << std::endl;
        tiny::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        std::cout << "  Tokenized: " << tokens.size() << " tokens" << std::endl;
        
        // 检查是否有词法错误
        for (const auto& token : tokens) {
            if (token.type == tiny::TokenType::Error) {
                std::cerr << "Lexical error at line " << token.line << ": " << token.lexeme << std::endl;
                return 1;
            }
        }
        
        // 2. 语法分析
        std::cout << "Phase 2: Parsing..." << std::endl;
        tiny::Parser parser(tokens);
        auto program = parser.parse();
        std::cout << "  Parsed: " << program->functions.size() << " function(s)" << std::endl;
        
        // 3. 代码生成
        std::cout << "Phase 3: Code Generation..." << std::endl;
        tiny::CodeGen codegen;
        
        // 设置输出文件名
        if (outputName.empty()) {
            outputName = fs::path(inputFile).stem().string();
        }
        
        bool success = codegen.compile(*program, outputName);
        if (!success) {
            std::cerr << "Code generation failed" << std::endl;
            return 1;
        }
        
        if (onlyIR) {
            std::cout << "\nGenerated LLVM IR:" << std::endl;
            std::cout << codegen.dumpIR() << std::endl;
        } else {
            // 链接生成可执行文件
            std::cout << "\nTo create executable, run:" << std::endl;
            std::cout << "  cc " << outputName << ".o runtime.c -o " << outputName << std::endl;
        }
        
        std::cout << "\nCompilation completed successfully!" << std::endl;
        return 0;
        
    } catch (const tiny::ParseError& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
