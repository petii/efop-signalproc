COMPILER := clang++
#COMPILER := g++
SRCDIR := src
BUILDDIR := build
DTARGET := bin/debug
TARGET := bin/release

SOURCES := $(shell find $(SRCDIR) -type f -name *.cpp)
CFLAGS := -std=c++11
LDFLAGS := -L${VULKAN_SDK}/lib `pkg-config --static --libs glfw3` -lvulkan -lportaudio
INCLUDE := -Iinclude -I${VULKAN_SDK}/include

all:
	make shaders
	echo $(SOURCES)
	$(COMPILER) -o visualizationapp.out $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DDEBUG -g -ggdb
	./visualizationapp.out
	make clean

shaders:
	cd src/shaders/ ; glslangValidator -V shader.*

clean:
	rm *.out
	rm src/shaders/*.spv
