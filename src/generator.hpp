// dummy code for testing

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include "parser.hpp"

class CodeGenerator {
private:
    std::ofstream outputFile;

    void writeHeader() {
        outputFile << "section .data\n";
        outputFile << "\nsection .text\n";
        outputFile << "global _start\n\n";
        outputFile << "_start:\n";
    }

    void writeFooter() {
        outputFile << "\n    ; Exit system call\n";
        outputFile << "    mov eax, 60\n";
        outputFile << "    xor edi, edi\n";
        outputFile << "    syscall\n";
    }

    void generateStatement(const std::string& statement) {
        if (statement.find("out") == 0) {
            outputFile << "    ; Output logic\n";
            outputFile << "    mov rax, 1\n";
            outputFile << "    mov rdi, 1\n";
            outputFile << "    mov rsi, msg\n";
            outputFile << "    mov rdx, msg_len\n";
            outputFile << "    syscall\n";
        } else if (statement.find("intbox") == 0) {
            outputFile << "    ; Variable declaration logic\n";
        } else {
            outputFile << "    ; Unhandled statement: " << statement << "\n";
        }
    }

public:
    CodeGenerator(const std::string& outputFileName) {
        outputFile.open(outputFileName);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Failed to open output file: " + outputFileName);
        }
    }

    ~CodeGenerator() {
        if (outputFile.is_open()) {
            outputFile.close();
        }
    }

    void generateCode(const std::vector<std::string>& statements) {
        writeHeader();
        for (const auto& statement : statements) {
            generateStatement(statement);
        }
        writeFooter();
    }
};
