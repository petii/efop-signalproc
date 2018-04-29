COMPILER := clang++
#COMPILER := g++
SRCDIR := src
BUILDDIR := build
DTARGET := bin/debug
TARGET := bin/release

SOURCES := $(shell find $(SRCDIR) -type f -name *.cpp)
CFLAGS := -std=c++11
LDFLAGS := -L${VULKAN_SDK}/lib `pkg-config --static --libs glfw3` -lvulkan
INCLUDE := -Iinclude -I${VULKAN_SDK}/include

compute:
	$(COMPILER) -o compute.out $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DDEBUG -g -ggdb

shaders:
	cd src/shaders/ ; glslangValidator -V shader.*

clean:
	rm *.out
