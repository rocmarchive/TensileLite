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
// Contains LDS ops
//

#include "Headers.h"

void getLgkmCnt(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, size_t count = 0);

void doDsWrite(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* dsPtr, llvm::Value *index, llvm::Value* val, size_t offset = 0);

llvm::Value* doDsRead(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* dsPtrA, llvm::Value *indexA, llvm::Value *dsPtrB, llvm::Value *indexB, size_t offset = 0);

void doDsRead(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Value* a0, llvm::Value* a1, llvm::Value* b0, llvm::Value* b1, llvm::Value* dsPtrA, llvm::Value *indexA, llvm::Value *dsPtrB, llvm::Value *indexB, size_t offset = 0);