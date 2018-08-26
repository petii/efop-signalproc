COMPILER := clang++
#COMPILER := g++
SRCDIR := src
BUILDDIR := build
DTARGET := bin/debug
TARGET := bin/release
OUTPUT_NAME := a.out

FFTWPP_PATH := /home/petii/.local/fftw++-2.05

SOURCES := $(shell find $(SRCDIR) -type f -name *.cpp) ${FFTWPP_PATH}/fftw++.cc
DEFINES := -D_REENTRANT
CFLAGS := -std=c++17 -fopenmp
LDFLAGS := -L${VULKAN_SDK}/lib -L/usr/local/lib `pkg-config --static --libs glfw3` -lvulkan -pthread -lportaudiocpp -lm -lpthread -lportaudio -lfftw3 -lfftw3_omp -lpulse
INCLUDE := -Iinclude -I${VULKAN_SDK}/include -I${FFTWPP_PATH} -I/usr/local/include 

all:
	make shaders
	make debug
	./$(DTARGET)/$(OUTPUT_NAME)
	make clean

debug:
	$(COMPILER) -o $(OUTPUT_NAME) $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(DEFINES) -DDEBUG -g -ggdb 

release:
	$(COMPILER) -o $(OUTPUT_NAME) $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -DNDEBUG -O3

shaders:
	cd src/shaders/ ; glslangValidator -V shader.comp 
	cd src/shaders/ ; glslangValidator -V hann.comp -o hann.spv 

clean:
	rm src/shaders/*.spv
