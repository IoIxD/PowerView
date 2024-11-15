set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.28)

project(PowerView)

set (CMAKE_CXX_STANDARD 20)



FILE(GLOB_RECURSE sources src/*.c src/*.cpp)


add_executable(${PROJECT_NAME} ${sources})

target_link_options(${PROJECT_NAME} PUBLIC -lGL -lglut)

