CXX:=clang++
LLVMFLAGS:=$(shell llvm-config --cxxflags --ldflags --system-libs --libs all)
CXXFLAGS:=-std=c++14 -g -fno-rtti
target:=kc
sources:=$(shell find . -iname '*.cpp')
objects:=$(addsuffix .o, $(basename $(sources)))

all: $(target)
	
$(target): $(objects)
	$(CXX) $(CXXFLAGS) $(LLVMFLAGS) -o $@ $^

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -f $(objects) $(target)
	
