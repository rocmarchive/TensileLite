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

#pragma once

//
// Contains functions for generating
// device memory load/store operations
//

#include "Headers.h"

// Provide C Matrix pointer, work item idx, idy, workgroup idx, idy, micro tile size, sub-micro tile size.
__attribute__((warning("Work in progress, expect API changes")))
std::vector<llvm::Value*> LoadCMatrixMicroTile(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, \
    tlite::types::tripleValue_t workitems, tlite::types::tripleValue_t workgroups, \
    tlite::types::tile_t matrix_dimensions, tlite::types::tile_t num_micro_tiles, \
    tlite::types::tile_t sub_micro_tile, tlite::types::tile_t workgroup_size, \
    tlite::types::dataType_t data_type, tlite::types::gfxArch_t gfx_arch);

__attribute__((warning("Work in progress, expect API changes")))
std::vector<llvm::Value*> LoadCMatrix(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Value* ptr, llvm::Value* workitem_index, std::vector<size_t> indices, std::vector<bool> is_relative_indices);

__attribute__((warning("Work in progress, expect API changes")))
void StoreCMatrix(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm::Value* ptr, llvm::Value* workitem_index, std::vector<size_t> indices, std::vector<bool> is_relative_indices);

void getGlobalLoad(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* ptr, llvm::Value *val, size_t offset=0);

llvm::Value* getGlobalLoad(llvm::IRBuilder<> &builder, llvm::Value* gep, llvm::Value *index, size_t offset=0);

llvm::Value* getGlobalLoad(llvm::IRBuilder<> &builder, llvm::Value* ptr, size_t offset = 0);

void getGlobalStore(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value *gep, llvm::Value *index, llvm::Value *value, size_t offset=0);

void getGlobalStore(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value *ptr, llvm::Value *value, size_t offset=0);

void getVmCnt(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, size_t count = 0);