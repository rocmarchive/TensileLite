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

#include"llvm/ADT/ArrayRef.h"
#include"llvm/IR/LLVMContext.h"
#include"llvm/IR/Module.h"
#include"llvm/IR/Function.h"
#include"llvm/IR/BasicBlock.h"
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

#include "globals.h"

namespace llvm {
  extern "C" void LLVMInitializeAMDGPUTargetInfo();
  extern "C" void LLVMInitializeAMDGPUTargetInfo();
  extern "C" void LLVMInitializeAMDGPUTarget();
  extern "C" void LLVMInitializeAMDGPUTargetMC();
  extern "C" void LLVMInitializeAMDGPUAsmParser();
  extern "C" void LLVMInitializeAMDGPUAsmPrinter();
}

using namespace llvm::sys;
using namespace llvm;

llvm::LLVMContext context;
llvm::Module *module = new llvm::Module("top", context);
llvm::IRBuilder<> builder(context);

std::map<std::string, llvm::Value*> NamedValues;

void generateFuncAST(){
  std::string funcName = "main";

  std::vector<std::string> Args;
  Args.push_back("a");
  Args.push_back("b");
  Args.push_back("c_load");
  Args.push_back("c_store");

  std::vector<llvm::Type*> argPointers(Args.size());

  for(uint32_t i=0;i<Args.size();i++){
    argPointers[i] = llvm::PointerType::get(llvm::VectorType::get(builder.getFloatTy(), 4), 0);
  }

  llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), argPointers, false);

  llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName.c_str(), module);

  mainFunc->setCallingConv(llvm::CallingConv::AMDGPU_KERNEL);

  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "", mainFunc);
  builder.SetInsertPoint(entry);

  uint32_t Idx = 0;
  for(auto &Arg: mainFunc->args()){
    std::string str = Args[Idx++];
    Arg.setName(str);
    NamedValues[str] = &Arg;
  }

  auto intrin_id = llvm::Intrinsic::amdgcn_workitem_id_x;
  auto f = llvm::Intrinsic::getDeclaration(module, intrin_id);
  auto tid_x = builder.CreateCall(f, {});
  intrin_id = llvm::Intrinsic::amdgcn_workitem_id_y;
  f = llvm::Intrinsic::getDeclaration(module, intrin_id);
  auto tid_y = builder.CreateCall(f, {});
  intrin_id = llvm::Intrinsic::amdgcn_workgroup_id_x;
  f = llvm::Intrinsic::getDeclaration(module, intrin_id);
  auto bid_x = builder.CreateCall(f, {});
  intrin_id = llvm::Intrinsic::amdgcn_workgroup_id_y;
  f = llvm::Intrinsic::getDeclaration(module, intrin_id);
  auto bid_y = builder.CreateCall(f, {});

  auto x0 = getInt(builder, context, 0);
  auto x16 = getInt(builder, context, 16);
  auto x2 = getInt(builder, context, 2);
  auto x1024 = getInt(builder, context, 1024);
  auto x32 = getInt(builder, context, 32);
  auto x64 = getInt(builder, context, 64);
  auto x4096 = getInt(builder, context, 4096);
  auto x8k = getInt(builder, context, 8192);
  auto x128k = getInt(builder, context, 128 * 1024);
  auto x64k = getInt(builder, context, 64*1024);

/*
  auto x0 = getInt(context, 0);
  auto x16 = getInt(context, 16);
  auto x2 = getInt(context, 2);
  auto x1024 = getInt(context, 1024);
  auto x32 = getInt(context, 32);
  auto x64 = getInt(context, 64);
  auto x4096 = getInt(context, 4096);
  auto x8k = getInt(context, 8192);
  auto x128k = getInt(context, 128 * 1024);
  auto x64k = getInt(context, 64*1024);
*/
  std::vector<llvm::Type*> argTypes1(1);
  std::vector<llvm::Value*> argValues1(1);
  argTypes1[0] = llvm::Type::getInt32Ty(context);
  argValues1[0] = tid_y;
  auto fTypetymod2 = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), argTypes1, false);
  auto tymod2 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "v_and_b32 $0, $1, 1 ; ty % 2", "=v,v", true), argValues1);
  auto tyby2 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "v_lshrrev_b32 $0, $1, 1 ; ty / 2", "=v,v", true), argValues1);
  argValues1[0] = tymod2;
  auto tymod2x16 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "v_lshlrev_b32 $0, $1, 4 ; (ty % 2) * 16", "=v,v", true), argValues1);
  argValues1[0] = tyby2;
  auto tyby2x1024 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "v_lshlrev_b32 $0, $1, 10 ; (ty / 2) * 1024", "=v,v", true), argValues1);
  auto tyby2x32 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "v_lshlrev_b32 $0, $1, 5 ; (ty / 2) * 32", "=v,v", true), argValues1);
  std::vector<llvm::Type*> argTypes2(2);
  std::vector<llvm::Value*> argValues2(2);
  argTypes2[0] = llvm::Type::getInt32Ty(context);
  argTypes2[1] = llvm::Type::getInt32Ty(context);
  argValues2[0] = tid_x;
  argValues2[1] = tymod2x16;
  auto fType2 = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), argTypes2, false);
  auto txtymod2x16 = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + (ty % 2) * 16","=v,v,v", true), argValues2);
  argValues2[0] = txtymod2x16;
  argValues2[1] = tyby2x32;
  auto id = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + (ty % 2) * 16 + (ty / 2) * 32", "=v,v,v", true), argValues2);

  argValues1[0] = bid_x;
  auto bxx32 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "s_lshl_b32 $0, $1, 5 ; bx * 32", "=s,s", true), argValues1);

  argValues1[0] = bid_y;
  auto byx32 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "s_lshl_b32 $0, $1, 5 ; by * 32", "=s,s", true), argValues1);

  argValues2[0] = txtymod2x16;
  argValues2[1] = tyby2x1024;
  auto txtymod2x16tyby2x1024 = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + (ty % 2) * 16 + (ty / 2) * 1024", "=v,v,v", true), argValues2);

  argValues2[0] = txtymod2x16tyby2x1024;
  argValues2[1] = byx32;
  auto a_global_id = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + (ty % 2) * 16 + (ty / 2) * 1024 + by * 32", "=v,v,s", true), argValues2);

  argValues2[1] = bxx32;
  auto b_global_id = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + (ty % 2) * 16 + (ty / 2) * 1024 + bx * 32", "=v,v,s", true), argValues2);

  argValues1[0] = tid_y;
  auto ty4k = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "v_lshlrev_b32 $0, $1, 12 ; ty * 4096", "=v,v", true), argValues1);

  argValues1[0] = bid_x;
  auto bx32 = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "s_lshl_b32 $0, $1, 5 ; bx * 32", "=s,s", true), argValues1);

  argValues1[0] = bid_y;
  auto by128k = builder.CreateCall(llvm::InlineAsm::get(fTypetymod2, "s_lshl_b32 $0, $1, 17 ; by * 128 * 1024", "=s,s", true), argValues1);

  argValues2[0] = tid_x;
  argValues2[1] = ty4k;
  auto txty4k = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + ty * 4096", "=v,v,v", true), argValues2);

  argValues2[0] = txty4k;
  argValues2[1] = bx32;
  auto txty4kbx32 = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + ty * 4096 + bx * 32", "=v,v,s", true), argValues2);

  argValues2[0] = txty4kbx32;
  argValues2[1] = by128k;
  auto c_global_id = builder.CreateCall(llvm::InlineAsm::get(fType2, "v_add_u32 $0, $1, $2 ; tx + ty * 4096 + bx * 32 + by * 128 * 1024", "=v,v,s", true), argValues2);


  auto A = NamedValues[Args[0]];
  auto B = NamedValues[Args[1]];

  llvm::Value* c0, *c1, *c2, *c3;
  llvm::Value* c4, *c5, *c6, *c7;
  llvm::Value* c8, *c9, *c10, *c11;
  llvm::Value* c12, *c13, *c14, *c15;

  auto floatType = llvm::Type::getFloatTy(context);
/*
  c0 = InitMicroTile(builder, context, floatType);
  c1 = InitMicroTile(builder, context, floatType);
  c2 = InitMicroTile(builder, context, floatType);
  c3 = InitMicroTile(builder, context, floatType);

  c4 = InitMicroTile(builder, context, floatType);
  c5 = InitMicroTile(builder, context, floatType);
  c6 = InitMicroTile(builder, context, floatType);
  c7 = InitMicroTile(builder, context, floatType);

  c8 = InitMicroTile(builder, context, floatType);
  c9 = InitMicroTile(builder, context, floatType);
  c10 = InitMicroTile(builder, context, floatType);
  c11 = InitMicroTile(builder, context, floatType);

  c12 = InitMicroTile(builder, context, floatType);
  c13 = InitMicroTile(builder, context, floatType);
  c14 = InitMicroTile(builder, context, floatType);
  c15 = InitMicroTile(builder, context, floatType);
*/
  auto C = IncrementPointer(builder, context, NamedValues[Args[2]], c_global_id);

  c0 = getGlobalLoad(builder, C);
  c4 = getGlobalLoad(builder, C, 256);

  auto C1 = IncrementPointer(builder, context, C, x1024, true);
  c1 = getGlobalLoad(builder, C1);
  c5 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointer(builder, context, C1, x1024, true);
  c2 = getGlobalLoad(builder, C1);
  c6 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointer(builder, context, C1, x1024, true);
  c3 = getGlobalLoad(builder, C1);
  c7 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointer(builder, context, C, x64k, true);
  c8 = getGlobalLoad(builder, C1);
  c12 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointer(builder, context, C1, x1024, true);
  c9 = getGlobalLoad(builder, C1);
  c13 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointer(builder, context, C1, x1024, true);
  c10 = getGlobalLoad(builder, C1);
  c14 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointer(builder, context, C1, x1024, true);
  c11 = getGlobalLoad(builder, C1);
  c15 = getGlobalLoad(builder, C1, 256);

/*
  auto C = IncrementPointerNoAsm(builder, context, NamedValues[Args[2]], c_global_id);

  c0 = getGlobalLoad(builder, C);
  c4 = getGlobalLoad(builder, C, 256);

  auto C1 = IncrementPointerNoAsm(builder, context, C, x1024);

  c1 = getGlobalLoad(builder, C1);
  c5 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointerNoAsm(builder, context, C1, x1024);
  c2 = getGlobalLoad(builder, C1);
  c6 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointerNoAsm(builder, context, C1, x1024);
  c3 = getGlobalLoad(builder, C1);
  c7 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointerNoAsm(builder, context, C, x64k);
  c8 = getGlobalLoad(builder, C1);
  c12 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointerNoAsm(builder, context, C1, x1024);
  c9 = getGlobalLoad(builder, C1);
  c13 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointerNoAsm(builder, context, C1, x1024);
  c10 = getGlobalLoad(builder, C1);
  c14 = getGlobalLoad(builder, C1, 256);

  C1 = IncrementPointerNoAsm(builder, context, C1, x1024);
  c11 = getGlobalLoad(builder, C1);
  c15 = getGlobalLoad(builder, C1, 256);
*/
  std::vector<unsigned> ids0(1, 0);
  std::vector<unsigned> ids1(1, 1);
  std::vector<unsigned> ids2(1, 2);
  std::vector<unsigned> ids3(1, 3);
  A = IncrementPointer(builder, context, A, a_global_id);
  B = IncrementPointer(builder, context, B, b_global_id);
  auto ra = getGlobalLoad(builder, A);
  auto rb = getGlobalLoad(builder, B);

  A = IncrementPointer(builder, context, A, x8k, true);
  B = IncrementPointer(builder, context, B, x8k, true);

  getVmCnt(builder, context, 0);
  doBarrier(builder, context);

//  auto id = doUMul(builder, doUAdd(builder, doUAdd(builder, tid_x, doUMul(builder, doURem(builder, tid_y, x2), x16)), doUMul(builder, doUDiv(builder, tid_y, x2), x32)), x16);

//  auto ldsA = x0;
  auto ldsB = x4096;

  std::vector<llvm::Type*> asmLdsTypes;
//  asmLdsTypes[0] = llvm::Type::getInt32Ty(context);
  std::vector<llvm::Value*> asmLdsValues;
  auto asmLdsFType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), asmLdsTypes, false);
  llvm::Value* ldsA = builder.CreateCall(llvm::InlineAsm::get(asmLdsFType, "s_mov_b32 $0, 0", "=s", true), asmLdsValues);

  auto ldsReadIdA = doUMul(builder, tid_y, x16);
  auto ldsReadIdB = doUMul(builder, tid_x, x16);

  llvm::Value *a0, *a1, *a2, *a3;
  llvm::Value *b0, *b1, *b2, *b3;
  llvm::Value *microTile0, *microTile1; // what gets loaded from lds 4 x float4, 2 from ldsA and 2 from ldsB

  a0 = InitMicroTile(builder, context, ra->getType());
  a1 = InitMicroTile(builder, context, ra->getType());
  a2 = InitMicroTile(builder, context, ra->getType());
  a3 = InitMicroTile(builder, context, ra->getType());

  b0 = InitMicroTile(builder, context, ra->getType());
  b1 = InitMicroTile(builder, context, ra->getType());
  b2 = InitMicroTile(builder, context, ra->getType());
  b3 = InitMicroTile(builder, context, ra->getType());


// shared_write<0>, shared_write<4096>
  doDsWrite(builder, context, ldsA, id, ra, 0);
  doDsWrite(builder, context, ldsA, id, rb, 4096);

// lgkmcnt<0>
  getLgkmCnt(builder, context, 0);
  doBarrier(builder, context);

  //microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, size_t(0));
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, size_t(0));

// for j = 1 -> (4096/8) 512

  auto Start = getInt(context, 1);
  auto TheFunction = builder.GetInsertBlock()->getParent();
  auto PreHeaderBB = builder.GetInsertBlock();

  auto LoopBB = llvm::BasicBlock::Create(context, "loop", TheFunction);
  builder.CreateBr(LoopBB);

  builder.SetInsertPoint(LoopBB);

  auto Variable = builder.CreatePHI(llvm::Type::getInt32Ty(context), 2, "");
  Variable->addIncoming(Start, PreHeaderBB);
  auto SetVal = getInt(context, 1);
  auto NextVar = builder.CreateAdd(Variable, SetVal);

// global_load<0>
  ra = getGlobalLoad(builder, A);
  rb = getGlobalLoad(builder, B);

  A = IncrementPointer(builder, context, A, x8k, true);
  B = IncrementPointer(builder, context, B, x8k, true);

// shared_read_b128<512>
//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

//
  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

// shared_read_b128<2*512>
//  microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 2*512);
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 2 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);


// shared_read_b128<3*512>
//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 3*512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 3 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

//
  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

// shared_read_b128<4*512>
//  microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 4*512);
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 4 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);


// shared_read_b128<5*512>
//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 5*512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA,  ldsB, ldsReadIdB, 5 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

//
  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

// shared_read_b128<6*512>
//  microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 6*512);
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 6 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);


//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 7*512);
//  ldsA = builder.CreateXor(ldsA, 0x2000);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 7 * 512);

  std::vector<llvm::Type*> xorArgTypes(1);
  xorArgTypes[0] = llvm::Type::getInt32Ty(context);
  auto asmXorFType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), xorArgTypes, false);

  std::vector<llvm::Value*> xorArgValues(1);
  xorArgValues[0] = ldsA;
  builder.CreateCall(llvm::InlineAsm::get(asmXorFType, "s_xor_b32 $0, $0, 0x2000", "s", true), xorArgValues, "");

  xorArgValues[0] = ldsB;
  builder.CreateCall(llvm::InlineAsm::get(asmXorFType, "s_xor_b32 $0, $0, 0x2000", "s", true), xorArgValues, "");

  ldsReadIdA = doUMul(builder, tid_y, x16);
  ldsReadIdB = doUMul(builder, tid_x, x16);

  getVmCnt(builder, context, 0);

  doDsWrite(builder, context, ldsA, id, ra, 0);
  doDsWrite(builder, context, ldsA, id, rb, 4096);

  getLgkmCnt(builder, context, 2);

  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

  getLgkmCnt(builder, context, 0);
  doBarrier(builder, context);

  //microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, size_t(0));
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, size_t(0));
 
  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);

// loop ends
  auto EndCond = builder.CreateICmpULT(NextVar, getInt(context, 512));
  auto LoopEndBB = builder.GetInsertBlock();
  auto AfterBB = llvm::BasicBlock::Create(context, "", TheFunction);
  builder.CreateCondBr(EndCond, LoopBB, AfterBB);

  builder.SetInsertPoint(AfterBB);

  Variable->addIncoming(NextVar, LoopEndBB);

// shared_read_b128<512>
//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

// shared_read_b128<2*512>
//  microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 2*512);
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 2 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);


// shared_read_b128<3*512>
//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 3*512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 3 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

// shared_read_b128<4*512>
//  microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 4*512);
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 4 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);


// shared_read_b128<5*512>
//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 5*512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 5 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

// shared_read_b128<6*512>
//  microTile0 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 6*512);
  doDsRead(builder, context, a0, a1, b0, b1, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 6 * 512);
// lgkmcnt<4>
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);


//  microTile1 = doDsRead(builder, context, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 7*512);
  doDsRead(builder, context, a2, a3, b2, b3, ldsA, ldsReadIdA, ldsB, ldsReadIdB, 7 * 512);
  getLgkmCnt(builder, context, 4);

  doMac4x4(builder, context, a0, b0, c0, c1, c2, c3);
  doMac4x4(builder, context, a0, b1, c4, c5, c6, c7);
  doMac4x4(builder, context, a1, b0, c8, c9, c10, c11);
  doMac4x4(builder, context, a1, b1, c12, c13, c14, c15);

  getLgkmCnt(builder, context, 0);

  doMac4x4(builder, context, a2, b2, c0, c1, c2, c3);
  doMac4x4(builder, context, a2, b3, c4, c5, c6, c7);
  doMac4x4(builder, context, a3, b2, c8, c9, c10, c11);
  doMac4x4(builder, context, a3, b3, c12, c13, c14, c15);

  C = IncrementPointer(builder, context, NamedValues[Args[3]], c_global_id);
  getGlobalStore(builder, context, C, c0);
  getGlobalStore(builder, context, C, c4, 256);

  auto C2 = IncrementPointer(builder, context, C, x1024);
  getGlobalStore(builder, context, C2, c1);
  getGlobalStore(builder, context, C2, c5, 256);

  C2 = IncrementPointer(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c2);
  getGlobalStore(builder, context, C2, c6, 256);

  C2 = IncrementPointer(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c3);
  getGlobalStore(builder, context, C2, c7, 256);

  C2 = IncrementPointer(builder, context, C, x64k);
  getGlobalStore(builder, context, C2, c8);
  getGlobalStore(builder, context, C2, c12, 256);

  C2 = IncrementPointer(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c9);
  getGlobalStore(builder, context, C2, c13, 256);

  C2 = IncrementPointer(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c10);
  getGlobalStore(builder, context, C2, c14, 256);

  C2 = IncrementPointer(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c11);
  getGlobalStore(builder, context, C2, c15, 256);


/*
  C = IncrementPointerNoAsm(builder, context, NamedValues[Args[3]], c_global_id);
  getGlobalStore(builder, context, C, c0);
  getGlobalStore(builder, context, C, c4, 256);

  auto C2 = IncrementPointerNoAsm(builder, context, C, x1024);
  getGlobalStore(builder, context, C2, c1);
  getGlobalStore(builder, context, C2, c5, 256);

  C2 = IncrementPointerNoAsm(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c2);
  getGlobalStore(builder, context, C2, c6, 256);

  C2 = IncrementPointerNoAsm(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c3);
  getGlobalStore(builder, context, C2, c7, 256);

  C2 = IncrementPointerNoAsm(builder, context, C, x64k);
  getGlobalStore(builder, context, C2, c8);
  getGlobalStore(builder, context, C2, c12, 256);

  C2 = IncrementPointerNoAsm(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c9);
  getGlobalStore(builder, context, C2, c13, 256);

  C2 = IncrementPointerNoAsm(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c10);
  getGlobalStore(builder, context, C2, c14, 256);

  C2 = IncrementPointerNoAsm(builder, context, C2, x1024);
  getGlobalStore(builder, context, C2, c11);
  getGlobalStore(builder, context, C2, c15, 256);
*/
/*
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id, c0);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id, c4, 256);

  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id1, c1);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id1, c5, 256);

  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id2, c2);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id2, c6, 256);

  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id3, c3);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id3, c7, 256);


  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id4, c8);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id4, c12, 256);

  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id5, c9);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id5, c13, 256);

  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id6, c10);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id6, c14, 256);

  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id7, c11);
  getGlobalStore(builder, context, NamedValues[Args[2]], c_global_id7, c15, 256);
*/
  getVmCnt(builder, context, 0);


  builder.CreateRetVoid();

  llvm::SmallString<8> data_ll;
  llvm::raw_svector_ostream dest_ll(data_ll);
  dest_ll.SetUnbuffered();
  std::string print_ll(data_ll.begin(), data_ll.end());
  module->print(errs(), nullptr);
  std::cout<<print_ll<<std::endl;

  auto TargetTriple = std::string("amdgcn-amd-amdhsa-hcc");

  llvm::LLVMInitializeAMDGPUTargetInfo();
  llvm::LLVMInitializeAMDGPUTarget();
  llvm::LLVMInitializeAMDGPUTargetMC();
  llvm::LLVMInitializeAMDGPUAsmParser();
  llvm::LLVMInitializeAMDGPUAsmPrinter();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  if(!Target){
    std::cout << Error<<std::endl;
    return;
  }

  auto GPU = "gfx900";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  auto targetMachine = Target->createTargetMachine(TargetTriple, GPU, Features, opt, RM);

  module->setDataLayout(targetMachine->createDataLayout());
  module->setTargetTriple(TargetTriple);

  auto FileName = "output.s";
  std::error_code EC;
  raw_fd_ostream dest(FileName, EC, sys::fs::F_None);

  llvm::legacy::PassManager pass;
  auto FileType = llvm::TargetMachine::CGFT_AssemblyFile;

  if(targetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    errs()<< "TargetMachine can't emit a file of this type";
  }

  pass.run(*module);
  dest.flush();

}

int main(){
  std::cout<<"; Generating AMDGPU LLVM IR"<<std::endl;
  generateFuncAST();
}

