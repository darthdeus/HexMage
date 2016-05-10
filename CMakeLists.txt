cmake_minimum_required(VERSION 3.5)
project(HexMage)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(include)
file(GLOB SOURCE_FILES src/*.cpp src/*.c)
#set(SOURCE_FILES
#    src/format.cpp
#    src/game.cpp
#    src/generator.cpp
#    src/gl_utils.cpp
#    src/glad.c
#    src/imgui.cpp
#    src/imgui_demo.cpp
#    src/imgui_draw.cpp
#    src/imgui_impl_sdl.cpp
#    src/input_manager.cpp
#    src/lodepng.cpp
#    src/main.cpp
#    src/model.cpp
#    src/simulation.cpp)

add_executable(HexMage ${SOURCE_FILES})

set(LIB_DIR c:/dev/HexMage/lib)
target_link_libraries(HexMage ${LIB_DIR}/SDL2.lib;${LIB_DIR}/SDL2main.lib;${LIB_DIR}/SDL2test.lib;${LIB_DIR}/freetype263.lib)
