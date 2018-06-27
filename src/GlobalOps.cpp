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

#include "tlite/GlobalOps.h"

void getGlobalLoad(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* ptr, llvm::Value *val, size_t offset) {
    std::vector<llvm::Type*> loadTypes(2);
    loadTypes[0] = val->getType();
    loadTypes[1] = ptr->getType();
    std::vector<llvm::Value*> loadValues(2);
    loadValues[0] = val;
    loadValues[1] = ptr;

    auto voidType = llvm::Type::getVoidTy(context);

    auto fTypeLoad = llvm::FunctionType::get(voidType, loadTypes, false);

    if(ptr->getType()->isPtrOrPtrVectorTy()) {
        if(!ptr->getType()->getPointerElementType()->isVectorTy()) {
        builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dword $0, $1, off offset:"+std::to_string(offset),"v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 1) {
        builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx2 $0, $1, off offset:"+std::to_string(offset),"v,v", true), loadValues, "");
    } 
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 2) {
        builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx2 $0, $1, off offset:"+std::to_string(offset),"v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 3) {
        builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx3 $0, $1, off offset:"+std::to_string(offset),"v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 4) {
        builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx4 $0, $1, off offset:"+std::to_string(offset),"v,v", true), loadValues, "");
    }
    }
}

llvm::Value* getGlobalLoad(llvm::IRBuilder<> &builder, llvm::Value* gep, llvm::Value *index, size_t offset) {
    std::vector<llvm::Value*> indices(1, index);
    auto ptr = builder.CreateGEP(gep, indices, llvm::Twine(index->getName()));
    
    std::vector<llvm::Type*> loadTypes(1, ptr->getType());
    std::vector<llvm::Value*> loadValues(1, ptr);
    auto fTypeLoad = llvm::FunctionType::get(ptr->getType()->getPointerElementType(), loadTypes, false);

    if(ptr->getType()->isPtrOrPtrVectorTy()) {
        if(!ptr->getType()->getPointerElementType()->isVectorTy()) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dword $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 1) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx2 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    } 
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 2) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx2 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 3) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx3 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 4) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx4 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    }
}


llvm::Value* getGlobalLoad(llvm::IRBuilder<> &builder, llvm::Value* ptr, size_t offset) {
    std::vector<llvm::Type*> loadTypes(1, ptr->getType());
    std::vector<llvm::Value*> loadValues(1, ptr);
    auto fTypeLoad = llvm::FunctionType::get(ptr->getType()->getPointerElementType(), loadTypes, false);

    if(ptr->getType()->isPtrOrPtrVectorTy()) {
        if(!ptr->getType()->getPointerElementType()->isVectorTy()) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dword $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 1) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx2 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    } 
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 2) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx2 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 3) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx3 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 4) {
        return builder.CreateCall(llvm::InlineAsm::get(fTypeLoad, "global_load_dwordx4 $0, $1, off offset:"+std::to_string(offset),"=v,v", true), loadValues, "");
    }
    }
}

void getGlobalStore(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value *gep, llvm::Value *index, llvm::Value *value, size_t offset) {
    std::vector<llvm::Value*> indices(1, index);
    auto ptr = builder.CreateGEP(gep, indices, llvm::Twine(index->getName()));

    std::vector<llvm::Type*> storeTypes(2);
    storeTypes[0] = value->getType();
    storeTypes[1] = ptr->getType();

    std::vector<llvm::Value*> storeValues(2);
    storeValues[0] = value;
    storeValues[1] = ptr;

    auto fTypeStore = llvm::FunctionType::get(llvm::Type::getVoidTy(context), storeTypes, false);

    if(ptr->getType()->isPtrOrPtrVectorTy()) {
        if(!ptr->getType()->getPointerElementType()->isVectorTy()) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dword $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 1) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx2 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        } 
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 2) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx2 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 3) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx3 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 4) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx4 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
    }
}

void getGlobalStore(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value *ptr, llvm::Value *value, size_t offset) {
    std::vector<llvm::Type*> storeTypes(2);
    storeTypes[0] = value->getType();
    storeTypes[1] = ptr->getType();

    std::vector<llvm::Value*> storeValues(2);
    storeValues[0] = value;
    storeValues[1] = ptr;

    auto fTypeStore = llvm::FunctionType::get(llvm::Type::getVoidTy(context), storeTypes, false);

    if(ptr->getType()->isPtrOrPtrVectorTy()) {
        if(!ptr->getType()->getPointerElementType()->isVectorTy()) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dword $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 1) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx2 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        } 
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 2) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx2 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 3) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx3 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
        else if(ptr->getType()->getPointerElementType()->getVectorNumElements() == 4) {
            builder.CreateCall(llvm::InlineAsm::get(fTypeStore, "global_store_dwordx4 $1, $0, off offset:"+std::to_string(offset),"v,v", true), storeValues, "");
        }
    }
}

void getVmCnt(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, size_t count) {
    std::vector<llvm::Value*> argValues;
    std::vector<llvm::Type*> argTypes;

    auto fTypeVmCnt = llvm::FunctionType::get(llvm::Type::getVoidTy(context), argTypes, false);

    builder.CreateCall(llvm::InlineAsm::get(fTypeVmCnt, "s_waitcnt vmcnt(" + std::to_string(count) + ")", "", true), argValues, "");
}