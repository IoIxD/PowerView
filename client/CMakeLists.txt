set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PowerView)

cmake_minimum_required(VERSION 3.28)

set (CMAKE_CXX_STANDARD 20)


FILE(GLOB_RECURSE sources src/*.c src/*.cpp)

add_executable(${PROJECT_NAME} ${sources})