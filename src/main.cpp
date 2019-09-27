#include "codegen.hpp"
#include "eval.hpp"
#include "parser.hpp"
#include <iostream>

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

    // evaluate regex
    std::cout << "\nresult: ";
    if (evalRegex(code, "abcdef")) {
        std::cout << "true" << std::endl;
    } else {
        std::cout << "false" << std::endl;
    }

    return 0;
}