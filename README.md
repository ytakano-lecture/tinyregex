# Tinyregex

## Setup for Ubuntu

```
$ sudo apt-get install -y build-essential clang git cmake llvm-dev lldb libncurses5-dev python3
```

## Source Code

The $(TINYREGEX)/src directory contains the skelton code of a tiny regular expression engine.

### Compilation and Execution

```
$ cd $(TINYREGEX)/jit
$ make
$ ./tinyregex regex file
```

## JIT Compilation with LLVM

The $(TINYREGEX)/jit directory contains an example of JIT compilation with LLVM.

### Compilation and Execution

```
$ cd $(TINYREGEX)/jit
$ cmake -DCMAKE_BUILD_TYPE=Debug .
$ make
$ ./tinyregex_jit
```