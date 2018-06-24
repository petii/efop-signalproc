COMPILER := clang++
#COMPILER := g++
SRCDIR := src
BUILDDIR := build
DTARGET := bin/debug
TARGET := bin/release
OUTPUT_NAME := vapp.out

SOURCES := $(shell find $(SRCDIR) -type f -name *.cpp)
CFLAGS := -std=c++17 -Wall
LDFLAGS := -L${VULKAN_SDK}/lib `pkg-config --static --libs glfw3` -lvulkan -lportaudio
INCLUDE := -Iinclude -I${VULKAN_SDK}/include

all:
	make shaders
	make debug
	./$(DTARGET)/$(OUTPUT_NAME)
	make clean

debug:
	$(COMPILER) -o $(DTARGET)/$(OUTPUT_NAME) $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DDEBUG -g -ggdb -Og

release:
	$(COMPILER) -o $(TARGET)/$(OUTPUT_NAME) $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DNDEBUG -O3

shaders:
	cd src/shaders/ ; glslangValidator -V shader.*

clean:
	rm $(DTARGET) $(TARGET) -r
	mkdir $(DTARGET) $(TARGET)
	rm src/shaders/*.spv
