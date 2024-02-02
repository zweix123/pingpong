cmake_minimum_required(VERSION 3.6)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(LLI)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(./src)