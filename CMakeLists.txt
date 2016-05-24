project(HexMage)
cmake_minimum_required(VERSION 2.8.12)

include_directories(include vendor/include HexMage.Simulator/include)
file(GLOB SOURCE_FILES src/*.cpp src/*.c vendor/src/*.c vendor/src/*.cpp)

add_subdirectory(HexMage.Simulator)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86)

if (MSVC)
	set(LIBS ${LIB_DIR}/SDL2.lib;${LIB_DIR}/SDL2main.lib;${LIB_DIR}/SDL2test.lib;${LIB_DIR}/freetype263.lib)  
endif (MSVC)

if (UNIX)
	set(LIBS SDL2 freetype)
	set(CUSTOM_FLAGS -std=c++14 -g)
endif (UNIX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CUSTOM_FLAGS}")

add_executable(HexMage ${SOURCE_FILES})
target_link_libraries(HexMage HexMage.Simulator.Lib ${LIBS})

add_custom_target(run
  COMMAND HexMage
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run HexMage"
  SOURCES ${SOURCE_FILES})
