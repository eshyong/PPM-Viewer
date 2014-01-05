CC=g++
CFLAGS=-c -g -Wall -I/usr/local/Cellar/sdl2/2.0.1/include
LDFLAGS=-L/usr/local/Cellar/sdl2/2.0.1/lib/ -lSDL2 -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL

all: clean ppm

ppm: ppm.o
	$(CC) $(LDFLAGS) ppm.o -o ppm

ppm.o:
	$(CC) $(CFLAGS) ppm.cpp

clean:
	rm -rf *o *.dSYM ppm
