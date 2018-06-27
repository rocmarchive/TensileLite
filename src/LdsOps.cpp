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

#include "tlite/LdsOps.h"
#include "tlite/Var.h"

void getLgkmCnt(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, size_t count) {
    std::vector<llvm::Value*> argValues;
    std::vector<llvm::Type*> argTypes;

    auto fTypeVmCnt = llvm::FunctionType::get(llvm::Type::getVoidTy(context), argTypes, false);

    builder.CreateCall(llvm::InlineAsm::get(fTypeVmCnt, "s_waitcnt lgkmcnt(" + std::to_string(count) + ")", "", true), argValues, "");
}

void doDsWrite(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* dsPtr, llvm::Value *index, llvm::Value* val, size_t offset) {
    auto ptr = doUAdd(builder, dsPtr, index);
    std::vector<llvm::Value*> writeValues(2);
    writeValues[0] = ptr;
    writeValues[1] = val;
    std::vector<llvm::Type*> writeTypes(2);
    writeTypes[0] = ptr->getType();
    writeTypes[1] = val->getType();
    auto fDsWriteType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), writeTypes, false);
    if(val->getType()->isVectorTy()) {
        if(val->getType()->getVectorNumElements() == 1) {
        builder.CreateCall(llvm::InlineAsm::get(fDsWriteType, "ds_write_b32 $0, $1 offset:"+std::to_string(offset), "v,v", true), writeValues, "");
        }
        else if(val->getType()->getVectorNumElements() == 2) {
        builder.CreateCall(llvm::InlineAsm::get(fDsWriteType, "ds_write_b64 $0, $1 offset:"+std::to_string(offset), "v,v", true), writeValues, "");
        }
        else if(val->getType()->getVectorNumElements() == 3) {
        builder.CreateCall(llvm::InlineAsm::get(fDsWriteType, "ds_write_b96 $0, $1 offset:"+std::to_string(offset), "v,v", true), writeValues, "");
        }
        else if(val->getType()->getVectorNumElements() == 4) {
        builder.CreateCall(llvm::InlineAsm::get(fDsWriteType, "ds_write_b128 $0, $1 offset:"+std::to_string(offset), "v,v", true), writeValues, "");
        }
    } else {
        builder.CreateCall(llvm::InlineAsm::get(fDsWriteType, "ds_write_b32 $0, $1 offset:"+std::to_string(offset), "v,v", true), writeValues, "");
    }
}

llvm::Value* doDsRead(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* dsPtrA, llvm::Value *indexA, llvm::Value *dsPtrB, llvm::Value *indexB, size_t offset) {
    auto ptra = doUAdd(builder, dsPtrA, indexA);
    auto ptrb = doUAdd(builder, dsPtrB, indexB);

    std::vector<llvm::Type*> vecTypes(4);
    auto scalarType = llvm::VectorType::get(llvm::Type::getFloatTy(context), 4);

    for(size_t i=0;i<4;i++) {
        vecTypes[i] = scalarType;
    }

    auto ABType = llvm::StructType::get(context, vecTypes);

    std::vector<llvm::Value*> readValues(2);

    readValues[0] = ptra;
    readValues[1] = ptrb;

    std::vector<llvm::Type*> readTypes(2);
    readTypes[0] = ptra->getType();
    readTypes[1] = ptrb->getType();

    auto fDsReadType = llvm::FunctionType::get(ABType, readTypes, false);

    return builder.CreateCall(llvm::InlineAsm::get(fDsReadType, std::string("ds_read_b128 $0, $4 offset:")+std::to_string(offset)+std::string("\nds_read_b128 $1, $4 offset:256+")+std::to_string(offset)+std::string("\nds_read_b128 $2, $5 offset:")+std::to_string(offset)+std::string("\nds_read_b128 $3, $5 offset:256+")+std::to_string(offset)+std::string("\n"),"=v,=v,=v,=v,v,v", true), readValues, "");
}

void doDsRead(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* a0, llvm::Value* a1, llvm::Value* b0, llvm::Value* b1, llvm::Value* dsPtrA, llvm::Value *indexA, llvm::Value *dsPtrB, llvm::Value *indexB, size_t offset) {
    auto ptra = doUAdd(builder, dsPtrA, indexA);
    auto ptrb = doUAdd(builder, dsPtrB, indexB);

    std::vector<llvm::Type*> vecTypes(4);
    auto scalarType = llvm::VectorType::get(llvm::Type::getFloatTy(context), 4);

    for(size_t i=0;i<4;i++) {
        vecTypes[i] = scalarType;
    }

    auto ABType = llvm::Type::getVoidTy(context);

    std::vector<llvm::Value*> readValues(2 + 4);
    uint64_t idx = 0;
    readValues[0] = a0;
    readValues[1] = a1;//builder.CreateExtractElement(microTile, idx++);
    readValues[2] = b0;//builder.CreateExtractElement(microTile, idx++);
    readValues[3] = b1;//builder.CreateExtractElement(microTile, idx++);
    readValues[4] = ptra;
    readValues[5] = ptrb;

    std::vector<llvm::Type*> readTypes(2 + 4);
    for(size_t i = 0; i < readTypes.size(); i++) {
        readTypes[i] = readValues[i]->getType();
    }

    auto fDsReadType = llvm::FunctionType::get(ABType, readTypes, false);

    builder.CreateCall(llvm::InlineAsm::get(fDsReadType, std::string("ds_read_b128 $0, $4 offset:")+std::to_string(offset)+std::string("\nds_read_b128 $1, $4 offset:256+")+std::to_string(offset)+std::string("\nds_read_b128 $2, $5 offset:")+std::to_string(offset)+std::string("\nds_read_b128 $3, $5 offset:256+")+std::to_string(offset)+std::string("\n"),"v,v,v,v,v,v", true), readValues, "");
}
