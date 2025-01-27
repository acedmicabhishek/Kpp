#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <optional>
#include "tokenstore.hpp"

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;

    // to get the current token
    Token& peek() {
        if (current < tokens.size())
            return tokens[current];
        throw std::runtime_error("Unexpected end of tokens");
    }

    Token& advance() {
        if (current < tokens.size())
            return tokens[current++];
        throw std::runtime_error("Unexpected end of tokens");
    }

    std::optional<Token> match(const std::string& type, const std::string& value = "") {
        if (current < tokens.size() && tokens[current].type == type && (value.empty() || tokens[current].value == value)) {
            return tokens[current++];
        }
        return std::nullopt;
    }

    // Parsing rules
    void parseProgram() {
        if (!match("keyword", "start"))
            throw std::runtime_error("Expected 'start' keyword at line " + std::to_string(peek().line));

        if (!match("symbol", "{"))
            throw std::runtime_error("Expected '{' after 'start' at line " + std::to_string(peek().line));

        while (!match("keyword", "close")) {
            parseStatement();
        }

        if (!match("symbol", "}"))
            throw std::runtime_error("Expected '}' after 'close' at line " + std::to_string(peek().line));
    }

    void parseStatement() {
        if (match("keyword", "intbox") || match("keyword", "floatbox") ||
            match("keyword", "stringbox") || match("keyword", "charbox") ||
            match("keyword", "boolbox")) {
            parseVariableDeclaration();
        } else if (match("keyword", "out")) {
            parseOutput();
        } else if (match("keyword", "in")) {
            parseInput();
        } else if (match("keyword", "if")) {
            parseIfStatement();
        } else {
            throw std::runtime_error("Unexpected statement at line " + std::to_string(peek().line));
        }
    }

    void parseVariableDeclaration() {
        if (!match("identifier"))
            throw std::runtime_error("Expected variable name at line " + std::to_string(peek().line));

        if (match("operator", "=")) {
            parseExpression();
        }

        if (!match("symbol", ";"))
            throw std::runtime_error("Expected ';' at the end of variable declaration at line " + std::to_string(peek().line));
    }

    void parseOutput() {
        // Handle '<<' operator after 'out'
        if (match("operator", "<") && match("operator", "<")) {
            parseExpression();
        } else {
            throw std::runtime_error("Expected '<<' after 'out' at line " + std::to_string(peek().line));
        }

        if (!match("symbol", ";"))
            throw std::runtime_error("Expected ';' at the end of output statement at line " + std::to_string(peek().line));
    }

    void parseInput() {
        if (!match("operator", ">>"))
            throw std::runtime_error("Expected '>>' after 'in' at line " + std::to_string(peek().line));

        if (!match("identifier"))
            throw std::runtime_error("Expected variable name after '>>' at line " + std::to_string(peek().line));

        if (!match("symbol", ";"))
            throw std::runtime_error("Expected ';' at the end of input statement at line " + std::to_string(peek().line));
    }

    void parseIfStatement() {
        parseExpression();

        if (!match("symbol", "{"))
            throw std::runtime_error("Expected '{' after 'if' condition at line " + std::to_string(peek().line));

        while (!match("symbol", "}")) {
            parseStatement();
        }

        if (match("keyword", "else")) {
            if (!match("symbol", "{"))
                throw std::runtime_error("Expected '{' after 'else' at line " + std::to_string(peek().line));

            while (!match("symbol", "}")) {
                parseStatement();
            }
        }
    }

    void parseExpression() {
        // Basic expression parsing (identifier, literals, operators)
        if (match("identifier") || match("integer_literal") || match("float_literal") ||
            match("string_literal") || match("char_literal")) {

            while (match("operator")) {
                if (!match("identifier") && !match("integer_literal") && !match("float_literal"))
                    throw std::runtime_error("Expected operand after operator at line " + std::to_string(peek().line));
            }
        } else {
            throw std::runtime_error("Invalid expression at line " + std::to_string(peek().line));
        }
    }

public:
    Parser(const std::vector<Token>& tokenStream) : tokens(tokenStream), current(0) {}

    void parse() {
        try {
            parseProgram();
            std::cout << "Parsing completed successfully.\n";
        } catch (const std::runtime_error& e) {
            std::cerr << "Parsing error: " << e.what() << "\n";
        }
    }
};
