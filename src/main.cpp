#include <iostream>
#include <fstream>
#include <filesystem>
#include "tokenstore.hpp"
#include "parser.hpp"
#include "generator.hpp"

int main(int argc, char* argv[]) {
    std::cout << "Compiler started\n";

    if (argc < 2) {
        std::cerr << "Usage: kat_compiler <file.kat>\n";
        return 1;
    }

    std::filesystem::path katFile = argv[1];
    if (katFile.extension() != ".kat") {
        std::cerr << "Error: Input file must have a .kat extension.\n";
        return 1;
    }

    std::ifstream file(katFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << katFile << "\n";
        return 1;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::cout << "Source code loaded successfully.\n";

    try {
        TokenStore tokenStore;
        tokenStore.tokenize(sourceCode);
        std::cout << "Tokenization completed successfully. Tokens:\n";
        tokenStore.printTokens();

        Parser parser(tokenStore.getTokens());
        parser.parse();

        NodeProg parsedProgram = parser.getParsedProgram();
        Generator codeGen("program.asm");

        codeGen.generateCode(parsedProgram.stmts);
        codeGen.finalize();

        std::cout << "Assembly code generated successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
