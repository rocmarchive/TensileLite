#!/bin/bash

FILENAME=test21
LLVM=llvm70

#~/${LLVM}/bin/clang++ ${FILENAME}.cpp `~/${LLVM}/bin/llvm-config --cxxflags --ldflags --system-libs --libs core all` -g -O3 -stdlib=libstdc++ -o ${FILENAME}

g++ ${FILENAME}.cpp `~/${LLVM}/bin/llvm-config --cxxflags --ldflags --system-libs --libs core all` -g -O0 -o ${FILENAME}
