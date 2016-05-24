project(HexMage.Simulator)

cmake_minimum_required(VERSION 2.8.12)

if (MSVC)

endif (MSVC)

if (UNIX)
	set(CUSTOM_FLAGS -std=c++14 -g)
endif (UNIX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CUSTOM_FLAGS}")

include_directories(include ../vendor/include)

file(GLOB SOURCE_FILES src/*.cpp src/*.c vendor/src/*.cpp vendor/src/*.c)

add_executable(HexMage.Simulator.Tests ${SOURCE_FILES} main.cpp)
add_library(HexMage.Simulator.Lib ${SOURCE_FILES})
