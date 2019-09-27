#include "eval.hpp"
#include <cassert>

static bool evalRegex(const std::vector<uint16_t> &code, const char *str,
                      uint32_t PC, uint32_t SP) {
    for (;;) {
        if (str[SP] == '\0') {
            return false;
        }

        switch (code[PC] & (3 << 14)) {
        case OPMATCH:
            // code: match
            // description: found
            return true;
        case OPCHAR: {
            // code: char c
            // description: if *SP != c then fail; else SP++ and CP++
            char c = (char)code[PC];
            if (c != str[SP]) {
                return false;
            } else {
                SP++;
                PC++;
                break;
            }
        }
        case OPJMP: {
            // code: jmp x
            // description: CP = x (jump to the address x)

            // implement here
            assert(false); // not yet implemented, remove this line

            break;
        }
        case OPSPLIT: {
            // code: split x, y
            // description: clone (one thread’s PC = x, and another’s PC = y)

            // implement here
            assert(false); // not yet implemented, remove this line

            break;
        }
        default:
            break;
        }
    }
}

bool evalRegex(const std::vector<uint16_t> &code, const char *str) {
    return evalRegex(code, str, 0, 0);
}