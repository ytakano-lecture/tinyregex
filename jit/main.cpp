#include <cctype>
#include <iostream>

#include "regexjit.hpp"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>

#define TRUEVAL(ctx) llvm::ConstantInt::get(ctx, llvm::APInt(1, 1, false))

llvm::LLVMContext llvmCtx;
std::unique_ptr<llvm::Module> llvmModule =
    std::make_unique<llvm::Module>("regexjit", llvmCtx);
llvm::IRBuilder<> llvmBuilder(llvmCtx);

// makeExample is an example of generating LLVM IR.
//
// C code:
// uint64_t __example(bool a, uint64_t b, uint64 c) {
//     if (a) {
//         return b + c;
//     } else {
//         return b * c;
//     }
// }
//
// LLVM IR:
// ; ModuleID = 'regexjit'
// source_filename = "regexjit"
//
// define i64 @__example(i1, i64, i64) {
// if:
//   %cond = icmp eq i1 %0, true
//   br i1 %cond, label %then, label %else
//
// then:                                             ; preds = %if
//   %add = add i64 %1, %2
//   br label %endif
//
// else:                                             ; preds = %if
//   %mul = mul i64 %1, %2
//   br label %endif
//
// endif:                                            ; preds = %else, %then
//   %result = phi i64 [ %add, %then ], [ %mul, %else ]
//   ret i64 %result
// }
llvm::Function *makeExample() {
    // create the type of arguments
    std::vector<llvm::Type *> argType;                   // type of arguments
    auto argType1 = llvm::IntegerType::get(llvmCtx, 1);  // boolean type
    auto argType2 = llvm::IntegerType::get(llvmCtx, 64); // 64 bit integer type
    auto argType3 = llvm::IntegerType::get(llvmCtx, 64); // 64 bit integer type

    argType.push_back(argType1);
    argType.push_back(argType2);
    argType.push_back(argType3);

    // create the type of the return value
    auto retType = llvm::IntegerType::get(llvmCtx, 64); // 64 bit integer type

    // create the type of the function
    auto funcType = llvm::FunctionType::get(retType, argType, false);

    // create the prototype of function
    auto funcDef = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "__example", *llvmModule);

    // get arguments
    std::vector<llvm::Value *> args;
    for (auto &v : funcDef->args()) {
        args.push_back(&v);
    }

    // create if block
    auto ifBlock = llvm::BasicBlock::Create(llvmCtx, "if", funcDef);
    auto thenBlock = llvm::BasicBlock::Create(llvmCtx, "then", funcDef);
    auto elseBlock = llvm::BasicBlock::Create(llvmCtx, "else", funcDef);
    auto endifBlock = llvm::BasicBlock::Create(llvmCtx, "endif", funcDef);

    // set the insertion point to the "if" block
    llvmBuilder.SetInsertPoint(ifBlock);

    // insert a compare equal
    auto cond = llvmBuilder.CreateICmpEQ(args[0], TRUEVAL(llvmCtx), "cond");

    // insert a conditional branch
    llvmBuilder.CreateCondBr(cond, thenBlock, elseBlock);

    // set the insertion point to the "then" block
    llvmBuilder.SetInsertPoint(thenBlock);

    // insert a add instruction
    auto addVal = llvmBuilder.CreateAdd(args[1], args[2], "add");

    // insert a branch instruction
    llvmBuilder.CreateBr(endifBlock);

    // set the insertion point to the "else" block
    llvmBuilder.SetInsertPoint(elseBlock);

    // insert a mul instruction
    auto mulVal = llvmBuilder.CreateMul(args[1], args[2], "mul");

    // insert a branch instruction
    llvmBuilder.CreateBr(endifBlock);

    // set the insertion point to the "endif" block
    llvmBuilder.SetInsertPoint(endifBlock);

    // insert a phi instruction
    auto result =
        llvmBuilder.CreatePHI(llvm::IntegerType::get(llvmCtx, 64), 2, "result");

    result->addIncoming(addVal, thenBlock);
    result->addIncoming(mulVal, elseBlock);

    // insert a return instruction
    llvmBuilder.CreateRet(result);

    return funcDef;
}

int main(int argc, char *argv[]) {
    // initialize
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // generate LLVM IR
    auto func = makeExample();

    // print LLVM IR
    std::string s;
    llvm::raw_string_ostream os(s);
    llvmModule->print(os, nullptr);
    std::cout << s << std::endl;

    // JIT compilation
    llvm::orc::RegexJIT jit;
    jit.addModule(std::move(llvmModule));

    // find address of the function
    auto symbol = jit.findSymbol("__example");
    auto ex = (uint64_t(*)(bool, uint64_t, uint64_t))(*symbol.getAddress());

    // call the function
    std::cout << "ex(false, 10, 20) = " << ex(false, 10, 20) << std::endl;
    std::cout << "ex(true, 10, 20) = " << ex(true, 10, 20) << std::endl;

    return 0;
}
