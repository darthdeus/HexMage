APPNAME   := bin/main
SOURCES		:= $(wildcard src/*.cpp src/*.c)
OBJECTS 	:= $(patsubst src%, out%, $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SOURCES))))

INCLUDE   := -I./include -I/usr/local/include -I../vendor/googletest/googletest/include -I../vendor/glm
LIBPATH		:= -L/usr/local/lib
LIBS			:= 

FLAGS			:= -O2 -g -fno-strict-aliasing
CCFLAGS 	:= $(FLAGS)
CXXFLAGS  := $(FLAGS) -std=c++14

CC        := clang
CXX       := clang++

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -o $(APPNAME) $(LIBPATH) $(LIBS)
	./bin/main

out/%.o: src/%.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@

out/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf out/*
	rm -f $(APPNAME)

# SRC=main.cpp glad.c model.cpp gl_utils.cpp src/*.cpp src/**/*.cpp
#
# CC=clang++
# OPTS=-c -Wall -std=c++14 -g -O0 $(INCLUDES)
# LIBS=-lsdl2 -L/usr/local/lib
#
# EXECUTABLE=bin/main

# all: $(EXECUTABLE)
#
# $(EXECUTABLE): $(OBJECTS)
# 	$(LINK.o) $^ -o $@ $(LIBS)
#
# clean:
# 	rm $(EXECUTABLE) $(OBJECTS)

# default:
# 	@clang++ $(CXXFLAGS) $(SRC) $(LIBS) -o bin/main
# 	@./bin/main
#
# %.o: %.cpp

# clean:
# 	@rm ./bin/main
