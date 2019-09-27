#include "codegen.hpp"
#include "eval.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>

int main(int argc, char *argv[]) {
    if (argc <= 2) {
        std::cout << "usage: " << argv[0] << " regex file" << std::endl;
        return 1;
    }

    // print regex
    std::cout << "regex: " << argv[1] << std::endl;

    // parse regex
    auto ast = parseRegex(argv[1]);
    if (ast == nullptr)
        return 1;

    // print AST
    std::cout << "\nabstract syntax tree:" << std::endl;
    printRegex(ast, 0);

    // generate labeled code
    auto lc = genLCode(ast);
    std::cout << "\nlabeled code:" << std::endl;
    printLCode(lc);

    // generate code
    auto code = genCode(lc);
    std::cout << "\ncode:" << std::endl;
    printCode(code);

    // open file
    std::ifstream ifs(argv[2]);
    std::string str;
    if (ifs.fail()) {
        std::cerr << "failed to open file: " << argv[2] << std::endl;
        return 2;
    }

    std::cout << "\nresult:" << std::endl;

    uint64_t line = 0;
    while (getline(ifs, str)) {
        line++;
        const char *p = str.c_str();
        while (*p != '\0') {
            // evaluate regex
            if (evalRegex(code, p)) {
                std::cout << line << ": " << str << std::endl;
                break;
            }
            p++;
        }
    }

    return 0;
}