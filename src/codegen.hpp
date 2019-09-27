#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "parser.hpp"

#include <set>

#define OPCHAR 0
#define OPJMP (1 << 14)
#define OPSPLIT (2 << 14)
#define OPMATCH (3 << 14)

// labeled machine code for regular expression
struct LCode {
    std::set<uint8_t> label;
    uint16_t code;
};

std::vector<LCode> genLCode(TRBase *expr);
std::vector<uint16_t> genCode(const std::vector<LCode> &lc);
void printLCode(const std::vector<LCode> &code);
void printCode(const std::vector<uint16_t> &code);

#endif // CODEGEN_HPP