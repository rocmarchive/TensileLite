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
// LLVM headers
//

#include"llvm/ADT/ArrayRef.h"
#include"llvm/IR/LLVMContext.h"
#include"llvm/IR/Module.h"
#include"llvm/IR/Function.h"
#include"llvm/IR/BasicBlock.h"
#include"llvm/IR/InlineAsm.h"
#include"llvm/IR/IRBuilder.h"
#include"llvm/IR/Constants.h"

#include"llvm/Support/TargetRegistry.h"
#include"llvm/Target/TargetOptions.h"
#include"llvm/Support/CodeGen.h"
#include"llvm/Target/TargetMachine.h"
#include"llvm/IR/LegacyPassManager.h"
#include"llvm/IR/PassManager.h"
#include"llvm/Support/FileSystem.h"
#include"llvm/Support/raw_ostream.h"

#include<system_error>

#include<iostream>
#include<vector>
#include<string>
#include<stdint.h>
#include<memory>
#include<map>
#include<fstream>