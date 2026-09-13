CXX := x86_64-elf-g++
LD := x86_64-elf-ld
HOSTCXX := g++
BIN := bin/notepad
OBJDIR := build/obj
SRCS := src/main.cpp src/str.cpp src/sys.cpp src/win.cpp src/docs.cpp src/files.cpp src/browser.cpp src/view.cpp src/input.cpp
OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)
CXXFLAGS := -g -O1 -ffreestanding -fno-stack-protector -fno-pic -m64 -mno-red-zone -mcmodel=small -std=c++20 -fno-exceptions -fno-rtti -I. -Iinclude -Isys -Igui -MMD -MP

all: $(BIN)

$(BIN): $(OBJS) linker.ld
	mkdir -p $(dir $@)
	$(LD) -T linker.ld -o $@ $(OBJS)

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: build/host_test
	build/host_test

build/host_test: test/host_test.cpp src/docs.cpp src/str.cpp
	mkdir -p build
	$(HOSTCXX) -std=c++20 -Iinclude -I. test/host_test.cpp src/docs.cpp src/str.cpp -o $@

-include $(OBJS:.o=.d)
