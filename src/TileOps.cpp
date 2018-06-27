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

#include "tlite/TileOps.h"

void doBarrier(llvm::IRBuilder<> &builder, llvm::LLVMContext &context) {
    std::vector<llvm::Type*> types;
    std::vector<llvm::Value*> values;

    auto fBarrier = llvm::FunctionType::get(llvm::Type::getVoidTy(context), types, false);
    builder.CreateCall(llvm::InlineAsm::get(fBarrier, "s_barrier","", true), values, "");
}


void doMac4x4(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* A, llvm::Value* B, llvm::Value* &C0, llvm::Value* &C1, llvm::Value* &C2, llvm::Value* &C3) {
    std::string asmStr = \ 
    "v_mac_f32 $0, $4, $5\n\tv_mac_f32 $1, $4, $6\n\tv_mac_f32 $2, $4, $7\n\tv_mac_f32 $3, $4, $8";

    std::string asmArgs = "v,v,v,v,v,v,v,v,v";

    std::vector<llvm::Value*> argValues(9);
    std::vector<llvm::Type*> argTypes(9);

    uint64_t Idx = 0;

    argValues[0] = builder.CreateExtractElement(C0, Idx++);
    argValues[1] = builder.CreateExtractElement(C0, Idx++);
    argValues[2] = builder.CreateExtractElement(C0, Idx++);
    argValues[3] = builder.CreateExtractElement(C0, Idx++);
    Idx = 0;
    argValues[5] = builder.CreateExtractElement(B, Idx++);
    argValues[6] = builder.CreateExtractElement(B, Idx++);
    argValues[7] = builder.CreateExtractElement(B, Idx++);
    argValues[8] = builder.CreateExtractElement(B, Idx++);
    Idx = 0;
    argValues[4] = builder.CreateExtractElement(A, Idx++);

    for(size_t i=0;i<argValues.size();i++) {
        argTypes[i] = argValues[i]->getType();
    }

    auto fTypeMac = llvm::FunctionType::get(llvm::Type::getVoidTy(context), argTypes, false);

    builder.CreateCall(llvm::InlineAsm::get(fTypeMac, asmStr, asmArgs, true), argValues, "");

    Idx = 1;
    argValues[4] = builder.CreateExtractElement(A, Idx);
    Idx = 0;
    argValues[0] = builder.CreateExtractElement(C1, Idx++);
    argValues[1] = builder.CreateExtractElement(C1, Idx++);
    argValues[2] = builder.CreateExtractElement(C1, Idx++);
    argValues[3] = builder.CreateExtractElement(C1, Idx++);

    builder.CreateCall(llvm::InlineAsm::get(fTypeMac, asmStr, asmArgs, true), argValues, "");

    Idx = 2;
    argValues[4] = builder.CreateExtractElement(A, Idx);
    Idx = 0;
    argValues[0] = builder.CreateExtractElement(C2, Idx++);
    argValues[1] = builder.CreateExtractElement(C2, Idx++);
    argValues[2] = builder.CreateExtractElement(C2, Idx++);
    argValues[3] = builder.CreateExtractElement(C2, Idx++);

    builder.CreateCall(llvm::InlineAsm::get(fTypeMac, asmStr, asmArgs, true), argValues, "");

    Idx = 3;
    argValues[4] = builder.CreateExtractElement(A, Idx++);
    Idx = 0;
    argValues[0] = builder.CreateExtractElement(C3, Idx++);
    argValues[1] = builder.CreateExtractElement(C3, Idx++);
    argValues[2] = builder.CreateExtractElement(C3, Idx++);
    argValues[3] = builder.CreateExtractElement(C3, Idx++);

    builder.CreateCall(llvm::InlineAsm::get(fTypeMac, asmStr, asmArgs, true), argValues, "");
}

void doAdd4x1(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* A, llvm::Value* B, llvm::Value* &C) {
    std::string asmStr = "v_add_f32 $0, $4, $8\nv_add_f32 $1, $5, $9\nv_add_f32 $2, $6, $10\nv_add_f32 $3, $7, $11";
    std::string asmArgs = "v,v,v,v, v,v,v,v, v,v,v,v";

    std::vector<llvm::Value*> argValues(12);
    std::vector<llvm::Type*> argTypes(12);

    uint64_t Idx = 0;
    argValues[0] = builder.CreateExtractElement(C, Idx++);
    argValues[1] = builder.CreateExtractElement(C, Idx++);
    argValues[2] = builder.CreateExtractElement(C, Idx++);
    argValues[3] = builder.CreateExtractElement(C, Idx++);
    Idx = 0;
    argValues[4] = builder.CreateExtractElement(A, Idx++);
    argValues[5] = builder.CreateExtractElement(A, Idx++);
    argValues[6] = builder.CreateExtractElement(A, Idx++);
    argValues[7] = builder.CreateExtractElement(A, Idx++);
    Idx = 0;
    argValues[8] = builder.CreateExtractElement(B, Idx++);
    argValues[9] = builder.CreateExtractElement(B, Idx++);
    argValues[10] = builder.CreateExtractElement(B, Idx++);
    argValues[11] = builder.CreateExtractElement(B, Idx++);

    for(size_t i=0;i<argValues.size();i++) {
        argTypes[i] = argValues[i]->getType();
    }

    auto fTypeAdd = llvm::FunctionType::get(llvm::Type::getVoidTy(context), argTypes, false);
    builder.CreateCall(llvm::InlineAsm::get(fTypeAdd, asmStr, asmArgs, true), argValues, "");

}


llvm::Value* InitMicroTile(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Type* element_type) {
    auto floatType = llvm::Type::getFloatTy(context);
    auto argType = llvm::Type::getVoidTy(context);

    std::vector<llvm::Value*> argValues;
    std::vector<llvm::Type*> argTypes;//(1, argType);

    std::vector<llvm::Type*> retTypes(4, floatType);

    auto retType = llvm::StructType::get(context, retTypes);

    auto fType = llvm::FunctionType::get(retType, argTypes, false);

    std::string asmStr = "v_mov_b32 $0, 0\n\
    v_mov_b32 $1, 0\n\
    v_mov_b32 $2, 0\n\
    v_mov_b32 $3, 0\n";

    std::string asmArgs = "=v,=v,=v,=v";

    auto vals = builder.CreateCall(llvm::InlineAsm::get(fType, asmStr, asmArgs, true), argValues);

    uint64_t idx = 0;
    auto v = builder.CreateExtractValue(vals, idx++);
    auto vecVals = builder.CreateVectorSplat(4, v);
    v = builder.CreateExtractValue(vals, idx++);
    vecVals = builder.CreateInsertElement(vecVals, v, 1);
    v = builder.CreateExtractValue(vals, idx++);
    vecVals = builder.CreateInsertElement(vecVals, v, 2);
    v = builder.CreateExtractValue(vals, idx++);
    vecVals = builder.CreateInsertElement(vecVals, v, 3);

    return vecVals;
/*
    std::vector<llvm::Value*> vec(4);
    std::vector<llvm::Value*> ret;

    uint64_t idx = 0;
    vec[0] = builder.CreateExtractElement(vals, idx++);
    vec[1] = builder.CreateExtractElement(vals, idx++);
    vec[2] = builder.CreateExtractElement(vals, idx++);
    vec[3] = builder.CreateExtractElement(vals, idx++);
    auto v = builder.CreateVectorSplat(4, vec[0]);
    v = builder.CreateInsertElement(v, vec[1], 1);
    v = builder.CreateInsertElement(v, vec[2], 2);
    v = builder.CreateInsertElement(v, vec[3], 3);
    ret.push_back(v);

    vec[0] = builder.CreateExtractElement(vals, idx++);
    vec[1] = builder.CreateExtractElement(vals, idx++);
    vec[2] = builder.CreateExtractElement(vals, idx++);
    vec[3] = builder.CreateExtractElement(vals, idx++);
    v = builder.CreateVectorSplat(4, vec[0]);
    v = builder.CreateInsertElement(v, vec[1], 1);
    v = builder.CreateInsertElement(v, vec[2], 2);
    v = builder.CreateInsertElement(v, vec[3], 3);
    ret.push_back(v);

    vec[0] = builder.CreateExtractElement(vals, idx++);
    vec[1] = builder.CreateExtractElement(vals, idx++);
    vec[2] = builder.CreateExtractElement(vals, idx++);
    vec[3] = builder.CreateExtractElement(vals, idx++);
    v = builder.CreateVectorSplat(4, vec[0]);
    v = builder.CreateInsertElement(v, vec[1], 1);
    v = builder.CreateInsertElement(v, vec[2], 2);
    v = builder.CreateInsertElement(v, vec[3], 3);
    ret.push_back(v);

    vec[0] = builder.CreateExtractElement(vals, idx++);
    vec[1] = builder.CreateExtractElement(vals, idx++);
    vec[2] = builder.CreateExtractElement(vals, idx++);
    vec[3] = builder.CreateExtractElement(vals, idx++);
    v = builder.CreateVectorSplat(4, vec[0]);
    v = builder.CreateInsertElement(v, vec[1], 1);
    v = builder.CreateInsertElement(v, vec[2], 2);
    v = builder.CreateInsertElement(v, vec[3], 3);
    ret.push_back(v);

/*
    std::string mov = "v_mov_b32 ";
    std::string asmStr;
    std::string asmArgs;
    for(size_t i = 0; i < 16; i++) {
        asmStr += (mov + "$" + std::to_string(i) + ", 0");
        if(i == 16 - 1) {
            asmArgs += "=v";
        } else {
            asmArgs += "=v, ";
        }
    }
*/

}