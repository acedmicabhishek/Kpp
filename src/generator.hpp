#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "parser.hpp"

class Generator {
private:
    std::ofstream outputFile;
    std::unordered_map<std::string, std::string> symbolTable;
    int tempVarCounter = 0;
    int labelCounter = 0;

    std::string getTempVar() {
        return "temp" + std::to_string(tempVarCounter++);
    }

    std::string getLabel(const std::string& base) {
        return base + std::to_string(labelCounter++);
    }

    void push(const std::string& reg) {
        outputFile << "    push " << reg << "\n";
    }

    void pop(const std::string& reg) {
        outputFile << "    pop " << reg << "\n";
    }

public:
    Generator(const std::string& outputFilePath) {
        outputFile.open(outputFilePath);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Failed to open output file: " + outputFilePath);
    
        outputFile << "section .data\n";
    }

    ~Generator() {
        if (outputFile.is_open()) {
            outputFile.close();
        }
    }

    void generateCode(const std::vector<ParsedStatement>& parsedStatements) {
        for (const auto& stmt : parsedStatements) {
            switch (stmt.type) {
                case StatementType::VariableDeclaration:
                    generateVariableDeclaration(stmt);
                    break;
                case StatementType::Output:
                    generateOutput(stmt);
                    break;
                case StatementType::Input:
                    generateInput(stmt);
                    break;
                case StatementType::IfStatement:
                    generateIfStatement(stmt);
                    break;
                case StatementType::WhileLoop:
                    generateWhileLoop(stmt);
                    break;
                case StatementType::Expression:
                    generateExpression(stmt);
                    break;
                default:
                    throw std::runtime_error("Invalid statement type");
            }
        }
    }

    void generateVariableDeclaration(const ParsedStatement& stmt) {
        std::string varType = stmt.tokens[0].value;
        std::string varName = stmt.tokens[1].value;
        std::string asmVar = "var_" + varName;
        symbolTable[varName] = asmVar;

        if (varType == "intbox") {
            outputFile << asmVar << " dd " << (stmt.tokens.size() > 3 ? stmt.tokens[3].value : "0") << "\n";
        } else if (varType == "floatbox") {
            outputFile << asmVar << " dq " << (stmt.tokens.size() > 3 ? stmt.tokens[3].value : "0.0") << "\n";
        } else if (varType == "charbox") {
            outputFile << asmVar << " db " << (stmt.tokens.size() > 3 ? "'" + stmt.tokens[3].value + "'" : "0") << "\n";
        } else if (varType == "stringbox") {
            outputFile << asmVar << " db " << (stmt.tokens.size() > 3 ? "\"" + stmt.tokens[3].value + "\"" : "\"\"") << ", 0\n";
        } else if (varType == "boolbox") {
            outputFile << asmVar << " db " << (stmt.tokens.size() > 3 ? (stmt.tokens[3].value == "true" ? "1" : "0") : "0") << "\n";
        } else {
            throw std::runtime_error("Unsupported variable type: " + varType);
        }
    }

    void generateOutput(const ParsedStatement& stmt) {
        outputFile << "section .text\n";
        outputFile << "    ; Output logic\n";
        for (const auto& token : stmt.tokens) {
            if (token.type == "string_literal") {
                outputFile << "    ; Print string literal\n";
                outputFile << "    mov rdi, " << token.value << "\n";
                outputFile << "    ; Add your OS-specific syscall for printing here\n";
            } else if (token.type == "identifier") {
                outputFile << "    ; Print identifier\n";
                outputFile << "    mov rax, " << symbolTable[token.value] << "\n";
                outputFile << "    ; Add your OS-specific syscall for printing here\n";
            } else if (token.type == "keyword" && token.value == "endl") {
                outputFile << "    ; Print newline\n";
                outputFile << "    mov rdi, '\\n'\n";
                outputFile << "    ; Add your OS-specific syscall for printing here\n";
            }
        }
    }

    void generateInput(const ParsedStatement& stmt) {
        // Placeholder for input generation logic
    }

    void generateIfStatement(const ParsedStatement& stmt) {
        std::string trueLabel = getLabel("true_branch");
        std::string falseLabel = getLabel("false_branch");
        std::string endLabel = getLabel("end_if");

        outputFile << "section .text\n";
        outputFile << "    ; If statement\n";

        const auto& condition = stmt.tokens;
        if (condition.size() == 3) {
            outputFile << "    cmp " << symbolTable[condition[0].value] << ", " << condition[2].value << "\n";
            if (condition[1].value == "==") {
                outputFile << "    je " << trueLabel << "\n";
            } else if (condition[1].value == "!=") {
                outputFile << "    jne " << trueLabel << "\n";
            } else if (condition[1].value == "<") {
                outputFile << "    jl " << trueLabel << "\n";
            } else if (condition[1].value == "<=") {
                outputFile << "    jle " << trueLabel << "\n";
            } else if (condition[1].value == ">") {
                outputFile << "    jg " << trueLabel << "\n";
            } else if (condition[1].value == ">=") {
                outputFile << "    jge " << trueLabel << "\n";
            }
        }

        outputFile << "    jmp " << falseLabel << "\n";
        outputFile << trueLabel << ":\n";

        // Generate code for true branch
        for (const auto& child : stmt.children) {
            generateCode({child});
        }

        outputFile << "    jmp " << endLabel << "\n";
        outputFile << falseLabel << ":\n";

        // Generate code for false branch
        if (stmt.children.size() > 1) {
            generateCode({stmt.children[1]});
        }

        outputFile << endLabel << ":\n";
    }

    void generateWhileLoop(const ParsedStatement& stmt) {
        std::string startLabel = getLabel("start_loop");
        std::string endLabel = getLabel("end_loop");

        outputFile << "section .text\n";
        outputFile << "    ; While loop\n";
        outputFile << startLabel << ":\n";

        const auto& condition = stmt.tokens;
        if (condition.size() == 3) {
            outputFile << "    cmp " << symbolTable[condition[0].value] << ", " << condition[2].value << "\n";
            if (condition[1].value == "==") {
                outputFile << "    je " << startLabel << "\n";
            } else if (condition[1].value == "!=") {
                outputFile << "    jne " << startLabel << "\n";
            } else if (condition[1].value == "<") {
                outputFile << "    jl " << startLabel << "\n";
            } else if (condition[1].value == "<=") {
                outputFile << "    jle " << startLabel << "\n";
            } else if (condition[1].value == ">") {
                outputFile << "    jg " << startLabel << "\n";
            } else if (condition[1].value == ">=") {
                outputFile << "    jge " << startLabel << "\n";
            }
        }
        for (const auto& child : stmt.children) {
            generateCode({child});
        }
        outputFile << "    jmp " << startLabel << "\n";
        outputFile << endLabel << ":\n";
    }

    void generateExpression(const ParsedStatement& stmt) {
        outputFile << "section .text\n";
        outputFile << "    ; Expression logic: ";
        for (const auto& token : stmt.tokens) {
            outputFile << token.value << " ";
        }
        outputFile << "\n";
    }

    void finalize() {
        outputFile << "    ; Finalize assembly\n";
        outputFile.close();
    }
};
