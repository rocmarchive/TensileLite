/*
Copyright (c) 2018 - present Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "tlite/Var.h"

llvm::Value* getInt(llvm::LLVMContext &context, uint64_t a) {
    return llvm::ConstantInt::get(context, llvm::APInt(32, a, false));
}

llvm::Value* getInt(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, uint64_t a, bool is_scalar) {
    std::vector<llvm::Type*> asmArgsType;
    std::vector<llvm::Value*> asmArgsValue;
    
    auto fType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), asmArgsType, false);

    if(is_scalar) {
    return builder.CreateCall(llvm::InlineAsm::get(fType, "s_mov_b32 $0, " + std::to_string(a), "=s", true), asmArgsValue, "");
    } else {
    return builder.CreateCall(llvm::InlineAsm::get(fType, "v_mov_b32 $0, " + std::to_string(a), "=v", true), asmArgsValue, "");
    }
}

llvm::Value* IncrementPointer(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Value* pointer, llvm::Value* inc, bool isIncScalar) {
    /*
    std::string asmStr = "v_add_co_u32 $0, vcc, $2, $4\n\t v_addc_co_u32 $1, vcc, $3, 0, vcc";
    std::string asmArgs = "=v,=v,v,v,v";
    */
    std::string asmStr = "v_add_co_u32 $0, vcc, $0, $2\n\t v_addc_co_u32 $1, vcc, $1, 0, vcc";
    std::string asmArgs;
    if(isIncScalar) {
        asmArgs = "v,v,s";
    } else {
        asmArgs = "v,v,v";
    }

    auto cast_to_int = builder.CreatePtrToInt(pointer, llvm::Type::getInt64Ty(context));
    auto trunc_lsb = builder.CreateTrunc(cast_to_int, llvm::Type::getInt32Ty(context));
    auto shift_ptr = builder.CreateLShr(cast_to_int, 32);
    auto trunc_msb = builder.CreateTrunc(shift_ptr, llvm::Type::getInt32Ty(context));
    /*
    std::vector<llvm::Type*> writeTypes(2);

    for(auto iter = writeTypes.begin(); iter != writeTypes.end(); iter++) {
        *iter = llvm::Type::getInt32Ty(context);
    }
    */

    std::vector<llvm::Type*> argTypes(3);
    argTypes[0] = llvm::Type::getInt32Ty(context);
    argTypes[1] = llvm::Type::getInt32Ty(context);
    argTypes[2] = inc->getType();
/*
    auto ABType = llvm::StructType::get(context, writeTypes);
    auto fTypeIncPtr = llvm::FunctionType::get(ABType, argTypes, false);
*/
    auto ABType = llvm::Type::getVoidTy(context);
    auto fTypeIncPtr = llvm::FunctionType::get(ABType, argTypes, false);
    std::vector<llvm::Value*> argValues(3);
    argValues[0] = trunc_lsb;
    argValues[1] = trunc_msb;
    argValues[2] = inc;
/*
    auto ret = builder.CreateCall(llvm::InlineAsm::get(fTypeIncPtr, asmStr, asmArgs, true), argValues, "");
    auto final_lsb = builder.CreateExtractValue(ret, uint64_t(0));
    auto final_msb = builder.CreateExtractValue(ret, 1);
    auto zext_lsb = builder.CreateZExt(final_lsb, llvm::Type::getInt64Ty(context));
    auto zext_msb = builder.CreateZExt(final_msb, llvm::Type::getInt64Ty(context));
    auto shl_msb = builder.CreateShl(zext_msb, 32, "", true, false);
    auto ptr = builder.CreateOr(zext_lsb, shl_msb);
    return builder.CreateIntToPtr(ptr, pointer->getType());
*/
    // returning void here. Convert back to pointer
    builder.CreateCall(llvm::InlineAsm::get(fTypeIncPtr, asmStr, asmArgs, true), argValues, "");
    auto zext_lsb = builder.CreateZExt(trunc_lsb, llvm::Type::getInt64Ty(context));
    auto zext_msb = builder.CreateZExt(trunc_msb, llvm::Type::getInt64Ty(context));
    auto shl_msb = builder.CreateShl(zext_msb, 32, "", true, false);
    auto ptr = builder.CreateOr(zext_lsb, shl_msb);

    return builder.CreateIntToPtr(ptr, pointer->getType());
}

llvm::Value* doUAdd(llvm::IRBuilder<> &builder, llvm::Value* LHS, llvm::Value* RHS, std::string varName) {
    return builder.CreateAdd(LHS, RHS, varName);
}

llvm::Value* doUMul(llvm::IRBuilder<> &builder, llvm::Value* LHS, llvm::Value *RHS, std::string varName) {
    return builder.CreateMul(LHS, RHS, varName);
}

llvm::Value* doUDiv(llvm::IRBuilder<> &builder, llvm::Value* LHS, llvm::Value *RHS, std::string varName) {
    return builder.CreateUDiv(LHS, RHS, varName);
}

llvm::Value* doURem(llvm::IRBuilder<> &builder, llvm::Value* LHS, llvm::Value *RHS, std::string varName) {
    return builder.CreateURem(LHS, RHS, varName);
}
