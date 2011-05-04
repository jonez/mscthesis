CC=gcc
FLAGS=-Wall -O3 -std=c99

SRCS=sources/*.c
LIBS=-lm -lOpenCL -lGLEW -lGLU -lglut
INCLUDES=-Iincludes/

EXEC=mc

all: build link clear

build:
	$(CC) $(FLAGS) -c $(SRCS) $(INCLUDES)

link:
	$(CC) *.o $(LIBS) -o $(EXEC)

clear:
	rm -rf *.o

