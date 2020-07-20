SRC=parser.cpp codegen.cpp eval.cpp main.cpp
HDR=parser.hpp codegen.hpp eval.hpp

all: tinyregex

tinyregex: $(SRC) $(HDR)
	clang++ -std=c++11 -g -O0 -o tinyregex $(SRC)

clean:
	rm -f tinyregex
