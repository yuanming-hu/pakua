cmake_minimum_required(VERSION 3.7)
project(research)

set(TAICHI_INCLUDE_DIR "$ENV{TAICHI_ROOT_DIR}/taichi/include")
message("Taichi include: ${TAICHI_INCLUDE_DIR}")

include_directories(${TAICHI_INCLUDE_DIR})
include_directories(include)
set(SOURCE_FILES server.cpp)
add_executable(research ${SOURCE_FILES})