CC = gcc
CFLAGS = -Wall -O3 -std=c99
FLAGS = $(CFLAGS) -pthread

USE_GL = -DUSE_GL=TRUE

CLIBS = -lm -lOpenCL
LIBS = $(CLIBS) -lGLEW -lGLU -lglut
INCLUDES = -Iincludes/

SRCS_DIR = sources
SRCS_FILES = $(SRCS_DIR)/*.c
KERNELS_DIR = kernels
UTILS_DIR = $(SRCS_DIR)/utils

CL_HELPER_SRC = clHelper

VIEWER = mcViewer
BENCHMARK = mcBenchmark
COMPILER = clCompiler


all: setup viewer benchmark compiler clear

setup:
	mkdir $(KERNELS_DIR)

viewer:
	$(CC) $(FLAGS) $(USE_GL) -c $(UTILS_DIR)/$(VIEWER).c $(SRCS_FILES) $(INCLUDES)
	$(CC) *.o $(LIBS) -o $(VIEWER)
	rm -f $(VIEWER).o

benchmark:
	$(CC) $(FLAGS) -c $(UTILS_DIR)/$(BENCHMARK).c $(SRCS_FILES) $(INCLUDES)
	$(CC) *.o $(CLIBS) -o $(BENCHMARK)
	rm -f $(BENCHMARK).o

compiler:
	$(CC) $(CFLAGS) -c $(UTILS_DIR)/$(COMPILER).c $(SRCS_DIR)/$(CL_HELPER_SRC).c $(INCLUDES)
	$(CC) *.o $(CLIBS) -o $(COMPILER)
	rm -f $(COMPILER).o

clear:
	rm -rf *.o
