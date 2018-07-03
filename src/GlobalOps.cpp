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


// 2 sets of register pairs
// one pair for storing C + index
// other pair to keep incrementing to do loads
// on different elements of micro-tile
std::vector<llvm::Value*> LoadCMatrix(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Value* ptr, llvm::Value* workitem_index, std::vector<size_t> indices, std::vector<bool> is_relative_indices) {
    std::vector<size_t> relative_indices(indices.size());
    std::vector<llvm::Value*> c_elements(indices.size() + 1);
    relative_indices[0] = indices[0];
    for(size_t i = 1; i < relative_indices.size(); i++) {
        if(is_relative_indices[i] == true) {
            relative_indices[i] = indices[i] - indices[i - 1];
        } else {
            relative_indices[i] = indices[i];
        }
    }
    for(auto iter = relative_indices.begin(); iter != relative_indices.end(); iter++) {
        std::cout << *iter << std::endl;
    }

    llvm::Value* c_ptr_to_int = builder.CreatePtrToInt(ptr, llvm::Type::getInt64Ty(context));
    size_t scale = sizeof(float) * 4;
    auto c_lsb = builder.CreateTrunc(c_ptr_to_int, llvm::Type::getInt32Ty(context));
    auto c_lshr = builder.CreateLShr(c_ptr_to_int, 32);
    auto c_msb = builder.CreateTrunc(c_lshr, llvm::Type::getInt32Ty(context));

    std::vector<llvm::Type*> ret_types_for_wiid(2);
    // wiid = workitem index
    std::vector<llvm::Type*> arg_types_for_wiid(3);
    std::vector<llvm::Value*> arg_values_for_wiid(3);
    auto scalar_type = llvm::Type::getInt32Ty(context);

    for(size_t i = 0; i < 2; i++) {
        ret_types_for_wiid[i] = scalar_type;
    }

    for(size_t i = 0; i < 3; i++) {
        arg_types_for_wiid[i] = scalar_type;
    }

    arg_values_for_wiid[0] = c_lsb;
    arg_values_for_wiid[1] = c_msb;
    arg_values_for_wiid[2] = workitem_index;

    std::string asm_str_wiid = "v_add_co_u32 $0, vcc, $2, $4\n v_addc_co_u32 $1, vcc, $3, 0, vcc\n";
    std::string asm_constraints_wiid = "=v,=v,v,v,v";

    auto ftype_wiid = llvm::FunctionType::get(llvm::StructType::get(context, ret_types_for_wiid), arg_types_for_wiid, false);

    auto ret_values_for_wiid = builder.CreateCall(llvm::InlineAsm::get(ftype_wiid, asm_str_wiid, asm_constraints_wiid, true), arg_values_for_wiid, "");

    uint64_t idx = 0;
    auto ptr_lsb_wiid = builder.CreateExtractValue(ret_values_for_wiid, idx++);
    auto ptr_msb_wiid = builder.CreateExtractValue(ret_values_for_wiid, idx++);

    auto ptr_lsb_wiid_64 = builder.CreateZExt(ptr_lsb_wiid, llvm::Type::getInt64Ty(context));
    auto ptr_msb_wiid_64 = builder.CreateZExt(ptr_msb_wiid, llvm::Type::getInt64Ty(context));
    auto ptr_msb_shl_wiid_64 = builder.CreateShl(ptr_msb_wiid_64, 32);
    auto ptr_wiid_64 = builder.CreateOr(ptr_lsb_wiid_64, ptr_msb_shl_wiid_64);

    auto ptr_wiid = builder.CreateIntToPtr(ptr_wiid_64, ptr->getType());

    // got first element
    c_elements[0] = builder.CreateLoad(ptr_wiid);


    // now, do the next element and keep using
    std::vector<llvm::Value*> arg_values_for_id(3);

    arg_values_for_id[0] = ptr_lsb_wiid;
    arg_values_for_id[1] = ptr_msb_wiid;
    arg_values_for_id[2] = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), relative_indices[0]);

    auto ret_values_for_id = builder.CreateCall(llvm::InlineAsm::get(ftype_wiid, asm_str_wiid, asm_constraints_wiid, true), arg_values_for_id, "");

    idx = 0;
    auto ptr_lsb_id = builder.CreateExtractValue(ret_values_for_id, idx++);
    auto ptr_msb_id = builder.CreateExtractValue(ret_values_for_id, idx++);

    auto ptr_lsb_id_64 = builder.CreateZExt(ptr_lsb_id, llvm::Type::getInt64Ty(context));
    auto ptr_msb_id_64 = builder.CreateZExt(ptr_msb_id, llvm::Type::getInt64Ty(context));
    auto ptr_msb_shl_id_64 = builder.CreateShl(ptr_msb_id_64, 32);
    auto ptr_id_64 = builder.CreateOr(ptr_lsb_id_64, ptr_msb_shl_id_64);

    auto ptr_id = builder.CreateIntToPtr(ptr_id_64, ptr->getType());

    c_elements[1] = builder.CreateLoad(ptr_id);

    std::string asm_str_id = "v_add_co_u32 $0, vcc, $0, $2\n v_addc_co_u32 $1, vcc, $1, 0, vcc\n";
    std::string asm_constraints_id = "v,v,v";

    auto ret_types_for_id = llvm::Type::getVoidTy(context);

    std::vector<llvm::Type*> arg_types_for_id(3);
    for(size_t i = 0; i < 3; i++) {
        arg_types_for_id[i] = scalar_type;
    }

    auto ftype_id = llvm::FunctionType::get(ret_types_for_id, arg_types_for_id, false);
    for(size_t i = 2; i < c_elements.size(); i++) {
        if(is_relative_indices[i-1] == true) {
        arg_values_for_id[0] = ptr_lsb_id;
        arg_values_for_id[1] = ptr_msb_id;
        arg_values_for_id[2] = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), relative_indices[i-1] * scale);
        builder.CreateCall(llvm::InlineAsm::get(ftype_id, asm_str_id, asm_constraints_id, true), arg_values_for_id, "");
        
        auto ptr_lsb_id_64_1 = builder.CreateZExt(ptr_lsb_id, llvm::Type::getInt64Ty(context));
        auto ptr_msb_id_64_1 = builder.CreateZExt(ptr_msb_id, llvm::Type::getInt64Ty(context));
        auto ptr_msb_shl_id_64_1 = builder.CreateShl(ptr_msb_id_64_1, 32);
        auto ptr_id_64_1 = builder.CreateOr(ptr_lsb_id_64_1, ptr_msb_shl_id_64_1);

        auto ptr_id = builder.CreateIntToPtr(ptr_id_64_1, ptr->getType());
        c_elements[i] = builder.CreateLoad(ptr_id);
        } else {
        arg_values_for_id[0] = ptr_lsb_wiid;
        arg_values_for_id[1] = ptr_msb_wiid;
        arg_values_for_id[2] = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), indices[i-1] * scale);
        ret_values_for_id = builder.CreateCall(llvm::InlineAsm::get(ftype_wiid, asm_str_wiid, asm_constraints_wiid, true), arg_values_for_wiid, "");

        idx = 0;
        ptr_lsb_id = builder.CreateExtractValue(ret_values_for_id, idx++);
        ptr_msb_id = builder.CreateExtractValue(ret_values_for_id, idx++);

        ptr_lsb_id_64 = builder.CreateZExt(ptr_lsb_id, llvm::Type::getInt64Ty(context));
        ptr_msb_id_64 = builder.CreateZExt(ptr_msb_id, llvm::Type::getInt64Ty(context));
        ptr_msb_shl_id_64 = builder.CreateShl(ptr_msb_id_64, 32);
        ptr_id_64 = builder.CreateOr(ptr_msb_shl_id_64, ptr_lsb_id_64);
        auto ptr_id = builder.CreateIntToPtr(ptr_id_64, ptr->getType());
        c_elements[i] = builder.CreateLoad(ptr_id);

        }
    }
    return c_elements;
}

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
