#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <variant>
#include "tokenstore.hpp"

enum class StatementType {
    VariableDeclaration,
    Output,
    Input,
    IfStatement,
    WhileLoop,
    Expression,
    Invalid
};

struct ParsedStatement {
    StatementType type;
    std::vector<Token> tokens;
    std::vector<ParsedStatement> children;
};

struct NodeProg {
    std::vector<ParsedStatement> stmts;
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    std::vector<ParsedStatement> parsedStatements;

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
            parsedStatements.push_back(parseStatement());
        }

        if (!match("symbol", "}"))
            throw std::runtime_error("Expected '}' after 'close' at line " + std::to_string(peek().line));
    }

    ParsedStatement parseStatement() {
        if (match("keyword", "intbox") || match("keyword", "floatbox") ||
            match("keyword", "stringbox") || match("keyword", "charbox") ||
            match("keyword", "boolbox")) {
            return parseVariableDeclaration();
        } else if (match("keyword", "out")) {
            return parseOutput();
        } else if (match("keyword", "in")) {
            return parseInput();
        } else if (match("keyword", "if")) {
            return parseIfStatement();
        } else if (match("keyword", "while")) {
            return parseWhileLoop();
        } else {
            throw std::runtime_error("Unexpected statement at line " + std::to_string(peek().line));
        }
    }

    ParsedStatement parseVariableDeclaration() {
        ParsedStatement stmt;
        stmt.type = StatementType::VariableDeclaration;
        stmt.tokens.push_back(tokens[current - 1]);

        if (!match("identifier"))
            throw std::runtime_error("Expected variable name at line " + std::to_string(peek().line));
        stmt.tokens.push_back(tokens[current - 1]);

        if (match("operator", "=")) {
            stmt.tokens.push_back(tokens[current - 1]);
            auto expression = parseExpression();
            stmt.tokens.insert(stmt.tokens.end(), expression.tokens.begin(), expression.tokens.end());
        }

        if (!match("symbol", ";"))
            throw std::runtime_error("Expected ';' at the end of variable declaration at line " + std::to_string(peek().line));

        return stmt;
    }

    ParsedStatement parseOutput() {
        ParsedStatement stmt;
        stmt.type = StatementType::Output;

        if (match("operator", "<") && match("operator", "<")) {
            stmt.tokens.push_back(tokens[current - 2]);
            auto expression = parseExpression();
            stmt.tokens.insert(stmt.tokens.end(), expression.tokens.begin(), expression.tokens.end());
        } else {
            throw std::runtime_error("Expected '<<' after 'out' at line " + std::to_string(peek().line));
        }

        if (!match("symbol", ";"))
            throw std::runtime_error("Expected ';' at the end of output statement at line " + std::to_string(peek().line));

        return stmt;
    }

    ParsedStatement parseInput() {
        ParsedStatement stmt;
        stmt.type = StatementType::Input;

        if (!match("operator", ">>"))
            throw std::runtime_error("Expected '>>' after 'in' at line " + std::to_string(peek().line));
        stmt.tokens.push_back(tokens[current - 1]); // Add the '>>' operator token

        if (!match("identifier"))
            throw std::runtime_error("Expected variable name after '>>' at line " + std::to_string(peek().line));
        stmt.tokens.push_back(tokens[current - 1]); // Add the identifier token

        if (!match("symbol", ";"))
            throw std::runtime_error("Expected ';' at the end of input statement at line " + std::to_string(peek().line));

        return stmt;
    }

    ParsedStatement parseIfStatement() {
        ParsedStatement stmt;
        stmt.type = StatementType::IfStatement;

        if (!match("symbol", "("))
            throw std::runtime_error("Expected '(' after 'if' at line " + std::to_string(peek().line));
        
        auto condition = parseExpression(); // Add the condition tokens
        stmt.tokens.insert(stmt.tokens.end(), condition.tokens.begin(), condition.tokens.end());

        if (!match("symbol", ")"))
            throw std::runtime_error("Expected ')' after condition at line " + std::to_string(peek().line));

        if (!match("symbol", "{"))
            throw std::runtime_error("Expected '{' after 'if' condition at line " + std::to_string(peek().line));

        while (!match("symbol", "}")) {
            stmt.children.push_back(parseStatement());
        }

        if (match("keyword", "else")) {
            if (!match("symbol", "{"))
                throw std::runtime_error("Expected '{' after 'else' at line " + std::to_string(peek().line));

            ParsedStatement elseStmt;
            elseStmt.type = StatementType::IfStatement;
            while (!match("symbol", "}")) {
                elseStmt.children.push_back(parseStatement());
            }
            stmt.children.push_back(elseStmt);
        }

        return stmt;
    }

    ParsedStatement parseWhileLoop() {
        ParsedStatement stmt;
        stmt.type = StatementType::WhileLoop;

        if (!match("symbol", "("))
            throw std::runtime_error("Expected '(' after 'while' at line " + std::to_string(peek().line));

        auto condition = parseExpression();
        stmt.tokens.insert(stmt.tokens.end(), condition.tokens.begin(), condition.tokens.end());

        if (!match("symbol", ")"))
            throw std::runtime_error("Expected ')' after condition at line " + std::to_string(peek().line));

        if (!match("symbol", "{"))
            throw std::runtime_error("Expected '{' after 'while' condition at line " + std::to_string(peek().line));

        while (!match("symbol", "}")) {
            stmt.children.push_back(parseStatement());
        }

        return stmt;
    }

    ParsedStatement parseExpression() {
        ParsedStatement stmt;
        stmt.type = StatementType::Expression;

        if (match("identifier") || match("integer_literal") || match("float_literal") ||
            match("string_literal") || match("char_literal") || match("keyword", "true") || match("keyword", "false") || match("keyword", "endl")) {
            stmt.tokens.push_back(tokens[current - 1]);

            while (match("operator")) {
                stmt.tokens.push_back(tokens[current - 1]);
                if (!match("identifier") && !match("integer_literal") && !match("float_literal") &&
                    !match("keyword", "true") && !match("keyword", "false") && !match("keyword", "endl"))
                    throw std::runtime_error("Expected operand after operator at line " + std::to_string(peek().line));
                stmt.tokens.push_back(tokens[current - 1]);
            }
        } else {
            throw std::runtime_error("Invalid expression at line " + std::to_string(peek().line));
        }

        return stmt;
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

    const std::vector<ParsedStatement>& getParsedStatements() const {
        return parsedStatements;
    }

    NodeProg getParsedProgram() const {
        NodeProg prog;
        prog.stmts = parsedStatements;
        return prog;
    }
};
