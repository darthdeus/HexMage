cmake_minimum_required(VERSION 3.5)
project(HexMage)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(include)
file(GLOB SOURCE_FILES src/*.cpp src/*.c)

include(build/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(HexMage ${SOURCE_FILES} ${CONAN_LIBS})

#set(LIB_DIR c:/dev/HexMage/lib)
#target_link_libraries(HexMage ${LIB_DIR}/SDL2.lib;${LIB_DIR}/SDL2main.lib;${LIB_DIR}/SDL2test.lib;${LIB_DIR}/freetype263.lib)
