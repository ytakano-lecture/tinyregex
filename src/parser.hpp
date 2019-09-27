#ifndef PARSER_HPP
#define PARSER_HPP

#include <cstdint>
#include <vector>

class TRBase {
  public:
    virtual ~TRBase(){};
};

class TRChar : public TRBase {
  public:
    char c;
};

class TROr : public TRBase {
  public:
    TRBase *left;
    TRBase *right;
};

class TRPlus : public TRBase {
  public:
    TRBase *expr;
};

class TRStar : public TRBase {
  public:
    TRBase *expr;
};

class TRQuestion : public TRBase {
  public:
    TRBase *expr;
};

class TRExprs : public TRBase {
  public:
    std::vector<TRBase *> exprs;
};

class TRMatch : public TRBase {};

TRBase *parseRegex(char *expr);
void printRegex(TRBase *expr, int indent);

#endif // PARSER_HPP