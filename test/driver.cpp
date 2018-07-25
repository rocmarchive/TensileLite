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

#include "tlite/Headers.h"

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
llvm::Module *module = new llvm::Module("", context);
llvm::IRBuilder<> builder(context);

std::map<std::string, llvm::Value*> NamedValues;

const std::string kstr_v_mac_f32_1x1x1 = "\tv_mac_f32 $0, $1, $2 \n";
const std::string kstr_v_mac_f32_regs = "v,v,v";

const std::string kstr_v_pk_mac_f16_2x1x2 = \
"\tv_pk_fma_f16 $0, $2, $3, $4 op_sel:[0,0,0] op_sel_hi:[0,1,0]\n\
\tv_pk_fma_f16 $1, $2, $3, $5 op_sel:[1,0,0] op_sel_hi:[1,1,0]\n";
const std::string kstr_v_pk_mac_f16_regs = "v,v,v,v,v,v"; // 6

llvm::Type* ktype_f32 = llvm::Type::getFloatTy(context);
llvm::Type* ktype_f16 = llvm::Type::getHalfTy(context);
llvm::Type* ktype_f16x2 = llvm::VectorType::get(ktype_f16, 2);

template<size_t gfx_number, size_t format>
int create_asm_block(size_t a_num, size_t b_num, size_t c_num) {

  std::vector<llvm::Type*> args_types(3);
  std::vector<llvm::Value*> args_values(3);

  llvm::FunctionType* device_func_type;
  llvm::Function* device_func;

  // fp32 += fp32 * fp32
  if(format == 0) {

    if(a_num + b_num + c_num > 256) {
      std::cerr << "The total number of registers exceeded 256, use lower numbers" << std::endl;
      return 0;
    }

    std::vector<llvm::Type*> asm_args_types(3, ktype_f32);
    std::vector<llvm::Value*> asm_args(3);

    std::string func_name = "tlite_mac_f32_f32_f32_" + std::to_string(a_num) + "_" + std::to_string(b_num) + "_" + std::to_string(c_num);
    args_types[0] = llvm::VectorType::get(llvm::Type::getFloatTy(context), a_num);
    args_types[1] = llvm::VectorType::get(llvm::Type::getFloatTy(context), b_num);
    args_types[2] = llvm::VectorType::get(llvm::Type::getFloatTy(context), c_num);

    device_func_type = llvm::FunctionType::get(builder.getVoidTy(), args_types, false);

    device_func = llvm::Function::Create(device_func_type, llvm::Function::ExternalLinkage, func_name, module);

    auto entry = llvm::BasicBlock::Create(context, "", device_func);
    builder.SetInsertPoint(entry);

    size_t args_values_id = 0;
    for(auto &Arg: device_func->args()) {
      args_values[args_values_id++] = &Arg;
    }

    llvm::Value* a_value = args_values[0], *b_value = args_values[1], *c_value = args_values[2];

    auto asm_func_type = llvm::FunctionType::get(builder.getVoidTy(), asm_args_types, false);

    

    for(size_t i = 0; i < b_num; i++) {
      for(size_t j = 0; j < a_num; j++) {
        auto a_element = builder.CreateExtractElement(a_value, i);
        auto b_element = builder.CreateExtractElement(b_value, j);

        auto c_element = builder.CreateExtractElement(c_value, i + j * a_num);

        asm_args[0] = c_element, asm_args[1] = a_element, asm_args[2] = b_element;

        builder.CreateCall(llvm::InlineAsm::get(asm_func_type, kstr_v_mac_f32_1x1x1, kstr_v_mac_f32_regs, true), asm_args, "");

      }
    }

  }

  if(format == 1) {

  if(a_num % 2 != 0 || b_num % 2 != 0) {
    std::cerr << "number of elements in a and b should be a multiple of 2" << std::endl;
    std::cerr << "odd number of half values will be supported later" << std::endl;
    return 0;
  }

    if(a_num + b_num + c_num > 256) {
      std::cerr << "The total number of registers exceeded 256, use lower numbers" << std::endl;
      return 0;
    }

    std::vector<llvm::Type*> asm_args_types(6, ktype_f16x2);
    std::vector<llvm::Value*> asm_args(6);

    std::string func_name = "tlite_pk_mac_f16_f16_f16_" + std::to_string(a_num) + "_" + std::to_string(b_num) + "_" + std::to_string(c_num);
    args_types[0] = llvm::VectorType::get(llvm::Type::getHalfTy(context), a_num);
    args_types[1] = llvm::VectorType::get(llvm::Type::getHalfTy(context), b_num);
    args_types[2] = llvm::VectorType::get(llvm::Type::getHalfTy(context), c_num);

    device_func_type = llvm::FunctionType::get(builder.getVoidTy(), args_types, false);

    device_func = llvm::Function::Create(device_func_type, llvm::Function::ExternalLinkage, func_name, module);

    auto entry = llvm::BasicBlock::Create(context, "", device_func);
    builder.SetInsertPoint(entry);

    size_t args_values_id = 0;
    for(auto &Arg: device_func->args()) {
      args_values[args_values_id++] = &Arg;
    }

    llvm::Value* a_value = args_values[0], *b_value = args_values[1], *c_value = args_values[2];

    auto asm_func_type = llvm::FunctionType::get(builder.getVoidTy(), asm_args_types, false);

    std::vector<uint32_t> int_mask = {0, 1};

    for(size_t i = 0; i < b_num; i=i+2) {
      for(size_t j = 0; j < a_num; j=j+2) {
        auto a_element_x = builder.CreateExtractElement(a_value, i);
        auto a_element_y = builder.CreateExtractElement(a_value, i+1);

        auto b_element_x = builder.CreateExtractElement(b_value, j);
        auto b_element_y = builder.CreateExtractElement(b_value, j+1);

        auto c_element_x = builder.CreateExtractElement(c_value, i + j * a_num);
        auto c_element_y = builder.CreateExtractElement(c_value, i + j * a_num + 1);

        auto c_element_z = builder.CreateExtractElement(c_value, i + j * a_num + b_num);
        auto c_element_w = builder.CreateExtractElement(c_value, i + j * a_num + b_num + 1);

        llvm::Value* a_element = builder.CreateVectorSplat(2, a_element_x);
        a_element = builder.CreateInsertElement(a_element, a_element_y, 1);
        
        llvm::Value* b_element = builder.CreateVectorSplat(2, b_element_x);
        b_element = builder.CreateInsertElement(b_element, b_element_y, 1);

        llvm::Value* c_element_1 = builder.CreateVectorSplat(2, c_element_x);
        c_element_1 = builder.CreateInsertElement(c_element_1, c_element_y, 1);

        llvm::Value* c_element_2 = builder.CreateVectorSplat(2, c_element_z);
        c_element_2 = builder.CreateInsertElement(c_element_2, c_element_w, 1);
        asm_args[0] = c_element_1, asm_args[1] = c_element_2, asm_args[2] = a_element, asm_args[3] = b_element, asm_args[4] = c_element_1, asm_args[5] = c_element_2;

        builder.CreateCall(llvm::InlineAsm::get(asm_func_type, kstr_v_pk_mac_f16_2x1x2, kstr_v_pk_mac_f16_regs, true), asm_args, "");

      }
    }
  }

  builder.CreateRetVoid();

}

int main(int argc, char* argv[]) {

  if(argc != 5) {
    std::cerr << "Usage: ./a.out <gfx number> <data format> <number of elements in a> <number of elements in b>" << std::endl;
    return 0;
  }

  size_t gfx_number = std::atoi(argv[1]);
  size_t data_format = std::atoi(argv[2]);
  size_t a_num = std::atoi(argv[3]);
  size_t b_num = std::atoi(argv[4]);

  size_t c_num = a_num * b_num;

  if(data_format == 0) {
    create_asm_block<803, 0>(a_num, b_num, c_num);
  } else if(data_format == 1) {
    create_asm_block<803, 1>(a_num, b_num, c_num);
  }

  llvm::SmallString<8> data_ll;
  llvm::raw_svector_ostream dest_ll(data_ll);
  dest_ll.SetUnbuffered();
  std::string print_ll(data_ll.begin(), data_ll.end());
  module->print(errs(), nullptr);
  std::cout<<print_ll<<std::endl;

}