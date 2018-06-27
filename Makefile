LLVM=~/llvm70
MC=$(LLVM)/bin/llvm-mc
LD=$(LLVM)/bin/ld.lld
TRIPLE=amdgcn--amdhsa-hcc
ARCH=amdgcn
CPU=gfx900
FILENAME=test21
HIPCC=/opt/rocm/bin/hipcc

CXX=g++
INCDIRS=./include
CFLAGS=-c -Wall `$(LLVM)/bin/llvm-config --cxxflags`
LDFLAGS=`$(LLVM)/bin/llvm-config --ldflags --system-libs --libs core all`
SOURCES=src/GlobalOps.cpp src/TileOps.cpp src/LdsOps.cpp src/Var.cpp test/$(FILENAME).cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=$(FILENAME)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) -I$(INCDIRS) $(CFLAGS) $< -o $@

mc: output.s
	$(MC) -arch=$(ARCH) -mcpu=$(CPU) -triple=$(TRIPLE) output.s -filetype=obj -o output.o

ld: output.o
	$(LD) -shared output.o -o output.co

rt: test/run_dup.cpp
	$(HIPCC) --amdgpu-target=gfx900 test/run_dup.cpp -o run

clean:
	rm -rf output.* *.o src/*.o test/*.o $(FILENAME) run
