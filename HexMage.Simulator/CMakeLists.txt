cmake_minimum_required(VERSION 3.5)
project(HexMage.Simulator)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(include ../vendor/glm ../vendor/googletest/googletest/include)
file(GLOB SOURCE_FILES src/*.cpp src/*.c)

include(build/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(HexMage.Simulator ${SOURCE_FILES})
target_link_libraries(HexMage.Simulator ${CONAN_LIBS})
