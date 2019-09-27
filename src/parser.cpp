#include "parser.hpp"
#include <cassert>
#include <iostream>
#include <typeindex>

static void printSpaces(int n) {
    for (int i = 0; i < n; i++)
        std::cout << " ";
}

static void printErr(const char *msg, char *expr, int pos) {
    std::cout << msg << "\n" << expr << std::endl;
    printSpaces(pos);
    std::cout << "^" << std::endl;
}

static bool isChar(char c) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
        ('0' <= c && c <= '9'))
        return true;
    return false;
}

static TRBase *makeUnary(char c, TRBase *expr) {
    switch (c) {
    case '+': {
        TRPlus *ret = new TRPlus;
        ret->expr = expr;
        return ret;
    }
    case '*': {
        TRStar *ret = new TRStar;
        ret->expr = expr;
        return ret;
    }
    case '?': {
        TRQuestion *ret = new TRQuestion;
        ret->expr = expr;
        return ret;
    }
    default:
        return nullptr;
    }
}

static TRBase *parseRegex(char *expr, int *pos, bool isParen) {
    TRExprs *ret = new TRExprs;

    for (;;) {
        switch (expr[*pos]) {
        case '\0':
            if (isParen) {
                // unmatched parenthesis, like "(ab", must be error
                printErr("error: unmatched parenthesis", expr, *pos);
                return nullptr;
            }

            // match
            ret->exprs.push_back(new TRMatch);
            return ret;
        case '(': {
            (*pos)++;
            TRBase *e = parseRegex(expr, pos, true);
            if (e == nullptr)
                return nullptr;

            ret->exprs.push_back(e);
            break;
        }
        case ')':
            if (isParen) {
                if (ret->exprs.empty()) {
                    // empty parenthesis, "()", must be error
                    printErr("error: empty expression", expr, *pos);
                    return nullptr;
                }
                (*pos)++;
                return ret;
            } else {
                // unmatched parenthesis, like "ab)", must be error
                printErr("error: unmatched parenthesis", expr, *pos);
                return nullptr;
            }
        case '|': {
            if (ret->exprs.empty()) {
                // no left expression, like "|ab", must be error
                printErr("error: no left expression", expr, *pos);
                return nullptr;
            }

            TROr *orexpr = new TROr;
            orexpr->left = ret;

            (*pos)++;
            TRBase *rhs = parseRegex(expr, pos, isParen);
            if (rhs == nullptr)
                return nullptr;

            orexpr->right = rhs;

            return orexpr;
        }
        case '+':
        case '*':
        case '?': {
            if (ret->exprs.empty()) {
                // no left expression, like "+" or ab(+cd), must be error
                printErr("error: no left expression", expr, *pos);
                return nullptr;
            }

            *ret->exprs.rbegin() = makeUnary(expr[*pos], *ret->exprs.rbegin());
            (*pos)++;
            break;
        }
        default:
            if (isChar(expr[*pos])) {
                TRChar *cexpr = new TRChar;
                cexpr->c = expr[*pos];
                ret->exprs.push_back(cexpr);
                (*pos)++;
            } else {
                printErr("error: invalid character", expr, *pos);
                return nullptr;
            }
            break;
        }
    }

    return ret;
}

void printRegex(TRBase *expr, int indent) {
    if (typeid(*expr) == typeid(TRChar)) {
        TRChar *e = dynamic_cast<TRChar *>(expr);
        assert(e);
        printSpaces(indent);
        std::cout << "char " << e->c << std::endl;
    } else if (typeid(*expr) == typeid(TRExprs)) {
        TRExprs *e = dynamic_cast<TRExprs *>(expr);
        assert(e);
        for (auto &p : e->exprs) {
            printRegex(p, indent);
        }
    } else if (typeid(*expr) == typeid(TROr)) {
        TROr *e = dynamic_cast<TROr *>(expr);
        assert(e);
        printSpaces(indent);
        std::cout << "|" << std::endl;
        printSpaces(indent);
        std::cout << "left:" << std::endl;
        printRegex(e->left, indent + 4);
        printSpaces(indent);
        std::cout << "right:" << std::endl;
        printRegex(e->right, indent + 4);
    } else if (typeid(*expr) == typeid(TRPlus)) {
        TRPlus *e = dynamic_cast<TRPlus *>(expr);
        assert(e);
        printSpaces(indent);
        std::cout << "+" << std::endl;
        printRegex(e->expr, indent + 4);
    } else if (typeid(*expr) == typeid(TRStar)) {
        TRStar *e = dynamic_cast<TRStar *>(expr);
        assert(e);
        printSpaces(indent);
        std::cout << "*" << std::endl;
        printRegex(e->expr, indent + 4);
    } else if (typeid(*expr) == typeid(TRQuestion)) {
        TRQuestion *e = dynamic_cast<TRQuestion *>(expr);
        assert(e);
        printSpaces(indent);
        std::cout << "?" << std::endl;
        printRegex(e->expr, indent + 4);
    } else if (typeid(*expr) == typeid(TRMatch)) {
        printSpaces(indent);
        std::cout << "match" << std::endl;
    }
}

TRBase *parseRegex(char *expr) {
    int pos = 0;
    return parseRegex(expr, &pos, false);
}