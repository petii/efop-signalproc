COMPILER := clang++
#COMPILER := g++
SRCDIR := src
BUILDDIR := build
DTARGET := bin/debug
TARGET := bin/release
OUTPUT_NAME := a.out

FFTWPP_PATH := /home/petii/.local/fftw++-2.05

SOURCES := $(shell find $(SRCDIR) -type f -name *.cpp) ${FFTWPP_PATH}/fftw++.cc
CFLAGS := -std=c++17 -fopenmp 
LDFLAGS := -L${VULKAN_SDK}/lib `pkg-config --static --libs glfw3` -lvulkan -lportaudio -lfftw3 -lfftw3_omp
INCLUDE := -Iinclude -I${VULKAN_SDK}/include -I${FFTWPP_PATH} 

all:
	make shaders
	make debug
	./$(DTARGET)/$(OUTPUT_NAME)
	make clean

debug:
	$(COMPILER) -o $(OUTPUT_NAME) $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DDEBUG -g -ggdb -Og

release:
	$(COMPILER) -o $(OUTPUT_NAME) $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DNDEBUG -O3

shaders:
	cd src/shaders/ ; glslangValidator -V shader.comp 
	cd src/shaders/ ; glslangValidator -V hann.comp -o hann.spv 

clean:
	rm src/shaders/*.spv
