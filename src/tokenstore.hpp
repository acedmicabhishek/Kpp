#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <regex>
#include <iostream>
#include <unordered_set>
#include <cctype>

struct Token {
    std::string type;
    std::string value;
    int line;
    int column;
};

class TokenStore {
private:
    std::vector<Token> tokens;
    int lineNumber;
    int columnNumber;

    const std::unordered_set<std::string> keywords = {
        "start", "close", "intbox", "floatbox", "stringbox", "charbox",
        "boolbox", "out", "in", "if", "else", "true", "false", "endl"
    };

    const std::unordered_set<std::string> operators = {
        "+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", "<<", ">>", "="
    };

    const std::unordered_set<char> symbols = {'{', '}', '(', ')', ';', ','};

    const std::regex identifierRegex = std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)");
    const std::regex integerRegex = std::regex(R"(\d+)");
    const std::regex floatRegex = std::regex(R"(\d+\.\d+)");
    const std::regex stringLiteralRegex = std::regex(R"("(\\.|[^"\\])*")");
    const std::regex charLiteralRegex = std::regex(R"('(\\.|[^'\\])')");

    void addToken(const std::string& type, const std::string& value) {
        tokens.push_back({type, value, lineNumber, columnNumber});
    }

    void skipComments(const std::string& source, size_t& pos) {
        if (source.substr(pos, 2) == "//") {
            while (pos < source.size() && source[pos] != '\n') pos++;
        } else if (source.substr(pos, 2) == "/*") {
            pos += 2; // Skip "/*"
            while (pos < source.size() && source.substr(pos, 2) != "*/") {
                if (source[pos] == '\n') {
                    lineNumber++;
                    columnNumber = 1;
                } else {
                    columnNumber++;
                }
                pos++;
            }
            if (pos >= source.size()) {
                throw std::runtime_error("Unterminated multi-line comment starting at line " +
                                         std::to_string(lineNumber));
            }
            pos += 2; // Skip "*/"
        }
    }

    void matchStringLiteral(const std::string& source, size_t& pos) {
        size_t start = pos++;
        while (pos < source.size() && source[pos] != '"') {
            if (source[pos] == '\\') pos++;
            pos++;
        }
        if (pos >= source.size() || source[pos] != '"') {
            throw std::runtime_error("Unterminated string literal at line " + std::to_string(lineNumber));
        }
        pos++;
        addToken("string_literal", source.substr(start, pos - start));
    }

    void matchCharLiteral(const std::string& source, size_t& pos) {
        size_t start = pos++;
        if (pos < source.size() && (source[pos] == '\\' || source[pos + 1] == '\'')) pos += 2;
        else pos++;
        if (pos >= source.size() || source[pos] != '\'') {
            throw std::runtime_error("Unterminated char literal at line " + std::to_string(lineNumber));
        }
        pos++;
        addToken("char_literal", source.substr(start, pos - start));
    }

    void matchNumber(const std::string& source, size_t& pos) {
        size_t start = pos;
        while (pos < source.size() && std::isdigit(source[pos])) pos++;
        if (pos < source.size() && source[pos] == '.') {
            pos++;
            while (pos < source.size() && std::isdigit(source[pos])) pos++;
            addToken("float_literal", source.substr(start, pos - start));
        } else {
            addToken("integer_literal", source.substr(start, pos - start));
        }
    }

public:
    TokenStore() : lineNumber(1), columnNumber(1) {}

    void tokenize(const std::string& source) {
        size_t pos = 0;
        size_t length = source.size();

        while (pos < length) {
            char currentChar = source[pos];

            if (isspace(currentChar)) {
                if (currentChar == '\n') {
                    lineNumber++;
                    columnNumber = 1;
                } else {
                    columnNumber++;
                }
                pos++;
                continue;
            }

            // Skip comments
            if (pos + 1 < length && (source.substr(pos, 2) == "//" || source.substr(pos, 2) == "/*")) {
                skipComments(source, pos);
                continue;
            }

            // Match keywords or identifiers
            if (std::isalpha(currentChar) || currentChar == '_') {
                size_t start = pos;
                while (pos < length && (std::isalnum(source[pos]) || source[pos] == '_')) pos++;
                std::string word = source.substr(start, pos - start);
                if (keywords.find(word) != keywords.end()) {
                    addToken("keyword", word);
                } else {
                    addToken("identifier", word);
                }
                continue;
            }

            // Match numbers
            if (std::isdigit(currentChar)) {
                matchNumber(source, pos);
                continue;
            }

            // Match string literals
            if (currentChar == '"') {
                matchStringLiteral(source, pos);
                continue;
            }

            // Match char literals
            if (currentChar == '\'') {
                matchCharLiteral(source, pos);
                continue;
            }

            // Match operators
            bool matchedOperator = false;
            for (const std::string& op : operators) {
                if (source.substr(pos, op.size()) == op) {
                    addToken("operator", op);
                    pos += op.size();
                    matchedOperator = true;
                    break;
                }
            }
            if (matchedOperator) continue;

            // Match symbols
            if (symbols.find(currentChar) != symbols.end()) {
                addToken("symbol", std::string(1, currentChar));
                pos++;
                continue;
            }

            // Unrecognized character
            throw std::runtime_error("Unknown token '" + std::string(1, currentChar) +
                                     "' at line " + std::to_string(lineNumber) +
                                     ", column " + std::to_string(columnNumber));
        }
    }

    const std::vector<Token>& getTokens() const {
        return tokens;
    }

    void printTokens() const {
        for (const auto& token : tokens) {
            std::cout << "Token(" << token.type << ", " << token.value
                      << ", line " << token.line << ", column " << token.column << ")\n";
        }
    }
};
