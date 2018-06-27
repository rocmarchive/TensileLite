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

#include <iostream>
#include <hip/hip_runtime_api.h>
#include <vector>
#include <fstream>
#include <chrono>

constexpr size_t M = 4096, N = 4096, K = 4096;
constexpr size_t size = M * N * sizeof(float);

#define CHECK(status) \
if(status != hipSuccess) {std::cerr<<"Got: "<<hipGetErrorString(status)<<" at: "<<__LINE__<<std::endl; }

#define fileName "output.co"
#define kernelName "main"

int main() {
    std::vector<float> A(M*K), B(K*N), C(M*N);
    std::fill(A.begin(), A.end(), 3.0f);
    std::fill(B.begin(), B.end(), 2.0f);
    std::fill(C.begin(), C.end(), -1.0f);

    hipDeviceptr_t Ad, Bd, Cd;
    CHECK(hipInit(0));
    hipDevice_t device;
    hipCtx_t context;
    CHECK(hipDeviceGet(&device, 0));
    CHECK(hipCtxCreate(&context, 0, device));

    CHECK(hipMalloc((void**)&Ad, size));
    CHECK(hipMalloc((void**)&Bd, size));
    CHECK(hipMalloc((void**)&Cd, size));

    CHECK(hipMemcpyHtoD(Ad, A.data(), size));
    CHECK(hipMemcpyHtoD(Bd, B.data(), size));
    CHECK(hipMemcpyHtoD(Cd, C.data(), size));

    hipModule_t Module;
    hipFunction_t Function;

    CHECK(hipModuleLoad(&Module, fileName));
    CHECK(hipModuleGetFunction(&Function, Module, kernelName));

    struct {
        void *Ad, *Bd, *CdLoad, *CdStore;
    } args;

    args.Ad = Ad;
    args.Bd = Bd;
    args.CdLoad = Cd;
    args.CdStore = Cd;

    size_t sizeArgs = sizeof(args);

    void *config[] = {HIP_LAUNCH_PARAM_BUFFER_POINTER, &args,
            HIP_LAUNCH_PARAM_BUFFER_SIZE, &sizeArgs,
            HIP_LAUNCH_PARAM_END};

    auto start = std::chrono::high_resolution_clock::now();
    CHECK(hipModuleLaunchKernel(Function, 32, 32, 1, 16, 16, 1, 128*8*2*2*sizeof(float), 0, NULL, (void**)&config));

    CHECK(hipDeviceSynchronize());
    auto stop = std::chrono::high_resolution_clock::now();

    CHECK(hipMemcpyDtoH(C.data(), Cd, size));

double sec = std::chrono::duration_cast<std::chrono::duration<double>>(stop - start).count();
    std::cout<<sec<<std::endl;
    double flops = (double)4096 * (double)4096 * (double)4096 * 2 / (double)1.0e12;
    double floppersec = flops / sec;
    std::cout<<flops<<" "<<sec<<" "<<floppersec<<std::endl;

std::ofstream outfile;
    outfile.open("outfile.txt");

    std::cout<<"writing to outfile"<<std::endl;

    for(int j=0;j<4096;j++) {
        for(int i=0;i<4096;i++) {
            outfile << C[i+j*4096] <<" ";
        }
        outfile <<"\n";
    }



    outfile<<"\n\n\n";

}


