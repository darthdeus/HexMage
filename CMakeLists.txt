project(HexMage)

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")

include_directories(include vendor/include HexMage.Simulator/include)
file(GLOB SOURCE_FILES src/*.cpp src/*.c vendor/src/*.c vendor/src/*.cpp)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()

add_subdirectory(HexMage.Simulator)

add_executable(HexMage ${SOURCE_FILES})
# target_link_libraries(HexMage ${CONAN_LIBS} HexMage.Simulator.Lib)
target_link_libraries(HexMage HexMage.Simulator.Lib SDL2 freetype)

add_custom_target(run
  COMMAND HexMage
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run HexMage"
  SOURCES ${SOURCE_FILES})

# add_dependencies(HexMage HexMage_run)

#set(LIB_DIR c:/dev/HexMage/lib)
#target_link_libraries(HexMage ${LIB_DIR}/SDL2.lib;${LIB_DIR}/SDL2main.lib;${LIB_DIR}/SDL2test.lib;${LIB_DIR}/freetype263.lib)
