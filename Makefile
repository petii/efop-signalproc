COMPILER := clang++
#COMPILER := g++
SRCDIR := src
BUILDDIR := build
DTARGET := bin/debug
TARGET := bin/release

SOURCES := $(shell find $(SRCDIR) -tye f -name *.cpp)
CFLAGS := -std=c++11
LDFLAGS := -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
INCLUDE := -Iinclude -I${VULKAN_SDK}/include

compute:
	$(COMPILER) -o compute.out $(SRCDIR)/main.cpp $(CFLAGS) $(INCLUDE) $(LDFLAGS)
