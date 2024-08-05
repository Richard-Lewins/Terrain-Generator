CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

LIBS = -lglfw -lGLU -lGL -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: main perlin_test structures_test

main: main.o structures.o terrain.o perlin.o
	$(CC) $(CFLAGS) -o main $^ $(LIBS)
perlin_test: perlin_test.o perlin.o structures.o testing.o
	$(CC) $(CFLAGS) -o perlin_test $^ $(LIBS)
structures_test: structures_test.o structures.o testing.o
	$(CC) $(CFLAGS) -o structures_test $^ $(LIBS)

main.o: main.c perlin.h structures.h terrain.h
structures.o: structures.c structures.h
terrain.o: terrain.c terrain.h
perlin.o: perlin.c perlin.h
testing.o: testing.c testing.h
perlin_test.o: perlin_test.c
structures_test.o: structures_test.c

clean:
	$(RM) *.o main perlin_test structures_test
	
