# TensileLite
LLVM inline ASM micro kernel generator

### Requirements
1. LLVM 7.0 with AMDGPU backend built
2. LLD installed

```sh
git clone https://github.com/llvm-mirror/llvm ~/llvm70_src
cd ~/llvm70_src/tools
git clone https://github.com/llvm-mirror/lld
cd ~/
mkdir llvm70_build_lnx
cd llvm70_build_lnx
cmake ../llvm70_src -DLLVM_TARGETS_TO_BUILD="AMDGPU" -DCMAKE_INSTALL_PREFIX=~/llvm70
make install -j `nproc`
```

Now, `llvm 7.0` is installed in `~/llvm70`

To build and run kernel,

```sh
make all
./test21
# edit output.s file to remove .amd_amdgpu_isa "amdgcn-amd-amdhsa-hcc-gfx900", will fix this soon
make mc
make ld
make rt
./run # to run validation test
```
