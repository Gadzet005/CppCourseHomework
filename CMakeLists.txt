cmake_minimum_required(VERSION 3.22)
project(CppHomework)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fsanitize=address -Wall)
add_link_options(-fsanitize=address)

file(GLOB SourceFiles src/*.cpp)
add_executable(main ${SourceFiles})
target_include_directories(main PRIVATE include)
