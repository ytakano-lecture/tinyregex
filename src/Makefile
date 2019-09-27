SRC=parser.cpp codegen.cpp eval.cpp main.cpp
HDR=parser.hpp codegen.hpp eval.hpp

all: tinyregex

tinyregex: $(SRC) $(HDR)
	clang++ -g -O0 -o tinyregex $(SRC)

clean:
	rm -f tinyregex