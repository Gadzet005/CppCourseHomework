cmake_minimum_required(VERSION 3.22)
project(CppHomework)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fsanitize=address -O2 -Wall)
add_link_options(-fsanitize=address)

add_executable(main main.cpp)