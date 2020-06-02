#include "codegen.hpp"
#include "parser.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <vector>

static uint8_t label;

static std::vector<LCode> genLCode(TRBase *expr, std::set<uint8_t> &nlabel);

static uint8_t nextLabel() {
    assert(label < 128);
    return label++;
}

// append v2 to v1
// ex:
//   input:
//     v1 = [1, 2]
//     v2 = [3, 4, 5]
//   output:
//     v1 = [1, 2, 3, 4, 5]
//     v2 = [3, 4, 5] // not modified
static void appendLCode(std::vector<LCode> &v1, const std::vector<LCode> &v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
}

// generete labeled code for "char"
static std::vector<LCode> genLChar(TRChar *e) {
    std::vector<LCode> ret;
    LCode c;

    c.code = (uint16_t)e->c; // machine code of "char"
    ret.push_back(c);

    return ret;
}

// generate labeled code for "match"
static std::vector<LCode> genLMatch() {
    std::vector<LCode> ret;
    LCode c;

    c.code = OPMATCH;
    ret.push_back(c);

    return ret;
}

// generate labeled code for expressions
static std::vector<LCode> genLExprs(TRExprs *e, std::set<uint8_t> &nlabel) {
    std::vector<LCode> ret;
    std::set<uint8_t> labels;

    for (auto &p : e->exprs) {
        std::set<uint8_t> nl;
        auto lc = genLCode(p, nl);
        assert(!lc.empty());

        // add the labels
        lc[0].label.insert(labels.begin(), labels.end());

        labels = nl;

        appendLCode(ret, lc); // concatenate
    }

    nlabel = labels;

    return ret;
}

// generete labeled code for "?"
// return:
//       split L1, L2
//   L1: codes for e
//   L2:
// output:
//   nlabel = {L2}
static std::vector<LCode> genLQuestion(TRQuestion *e,
                                       std::set<uint8_t> &nlabel) {
    std::vector<LCode> ret;

    // generate labels for split
    uint8_t L1 = nextLabel(), L2 = nextLabel();

    // split L1, L2
    LCode split;
    split.code = OPSPLIT | L1 << 7 | L2; // machine code of split

    ret.push_back(split); // append "split" to the last

    // L1: codes for e
    auto lc = genLCode(e->expr, nlabel);
    assert(!lc.empty());
    lc[0].label.insert(L1);

    appendLCode(ret, lc);

    // L2:
    nlabel.insert(L2); // a label of the next code

    return ret;
}

// generate labeled code
static std::vector<LCode> genLCode(TRBase *expr, std::set<uint8_t> &nlabel) {
    if (typeid(*expr) == typeid(TRExprs)) {
        auto *e = dynamic_cast<TRExprs *>(expr);
        return genLExprs(e, nlabel);
    } else if (typeid(*expr) == typeid(TRChar)) {
        auto *e = dynamic_cast<TRChar *>(expr);
        return genLChar(e);
    } else if (typeid(*expr) == typeid(TRQuestion)) {
        auto *e = dynamic_cast<TRQuestion *>(expr);
        return genLQuestion(e, nlabel);
    } else if (typeid(*expr) == typeid(TRMatch)) {
        return genLMatch();
    } else if (typeid(*expr) == typeid(TRPlus)) {
        // implement here
    } else if (typeid(*expr) == typeid(TRStar)) {
        // implement here
    } else if (typeid(*expr) == typeid(TROr)) {
        // implement here
    }

    assert(false); // never reach here if every operation is implemented
    return std::vector<LCode>();
}

std::vector<LCode> genLCode(TRBase *expr) {
    std::set<uint8_t> labels;
    return genLCode(expr, labels);
}

// generate code from labeled code
std::vector<uint16_t> genCode(const std::vector<LCode> &lc) {
    std::vector<uint16_t> ret;
    std::map<uint8_t, uint8_t> label2addr;

    // make a map from labels to addresses
    for (int i = 0; i < lc.size(); i++) {
        for (auto &label : lc[i].label) {
            label2addr[label] = i;
        }
    }

    for (auto &c : lc) {
        switch (c.code & (3 << 14)) {
        case OPMATCH:
        case OPCHAR:
            // "match" and "char" do not require translation
            ret.push_back(c.code);
            break;
        case OPJMP: {
            // translate the label to corresponding address
            uint16_t addr = c.code & 0x03fff;
            addr = c.code & label2addr[addr];
            ret.push_back(OPJMP | addr);
            break;
        }
        case OPSPLIT: {
            // translate the label to corresponding address

            // implement here

            break;
        }
        default:
            assert(false); // never reach here
            break;
        }
    }

    return ret;
}

// print labeled code
void printLCode(const std::vector<LCode> &code) {
    for (auto &c : code) {
        int n = 0;
        for (auto &L : c.label) {
            if (n > 0)
                std::cout << ", ";
            std::cout << "L" << (uint32_t)L;
            n++;
        }
        if (n > 0)
            std::cout << ":\n";

        switch (c.code & (3 << 14)) {
        case OPMATCH:
            std::cout << "  match" << std::endl;
            break;
        case OPCHAR: {
            std::cout << "  char " << (char)c.code << std::endl;
            break;
        }
        case OPSPLIT: {
            uint8_t L1 = c.code >> 7, L2 = c.code & 0x007f;
            std::cout << "  split L" << (uint32_t)L1 << ", L" << (uint32_t)L2
                      << std::endl;
            break;
        }
        case OPJMP: {
            uint16_t L = c.code & 0x3fff;
            std::cout << "  jmp L" << L << std::endl;
            break;
        }
        default:
            assert(false); // never reach here
            break;
        }
    }
}

static void printDigit4(int n) {
    if (n < 10) {
        std::cout << "   " << n;
    } else if (n < 100) {
        std::cout << "  " << n;
    } else if (n < 1000) {
        std::cout << " " << n;
    } else {
        std::cout << n;
    }
}

// print code
void printCode(const std::vector<uint16_t> &code) {
    int n = 0;
    for (auto &c : code) {
        switch (c & (3 << 14)) {
        case OPMATCH:
            printDigit4(n);
            std::cout << "  match" << std::endl;
            break;
        case OPCHAR: {
            printDigit4(n);
            std::cout << "  char " << (char)c << std::endl;
            break;
        }
        case OPSPLIT: {
            uint8_t L1 = c >> 7, L2 = c & 0x007f;
            printDigit4(n);
            std::cout << "  split " << (uint32_t)L1 << ", " << (uint32_t)L2
                      << std::endl;
            break;
        }
        case OPJMP: {
            uint16_t L = c & 0x3fff;
            printDigit4(n);
            std::cout << "  jmp L" << L << std::endl;
            break;
        }
        default:
            assert(false); // never reach here
            break;
        }
        n++;
    }
}