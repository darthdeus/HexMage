LIBS=-lsdl2
CXXFLAGS=-O0 -g -std=c++14 -I./include -I/usr/local/include -L/usr/local/lib
SRC=main.cpp glad.c

default:
	@clang++ $(CXXFLAGS) $(SRC) $(LIBS) -o bin/main
	@./bin/main
