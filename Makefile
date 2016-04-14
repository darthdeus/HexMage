LIBS=-lsdl2
INCLUDES=-I./include -I/usr/local/include
CXXFLAGS=-O0 -g -std=c++14 $(INCLUDES) -L/usr/local/lib
SRC=main.cpp glad.c mob.cpp gl_utils.cpp

default:
	@clang++ $(CXXFLAGS) $(SRC) $(LIBS) -o bin/main
	@./bin/main

clean:
	@rm ./bin/main
