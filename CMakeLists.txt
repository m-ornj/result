cmake_minimum_required(VERSION 3.21)
project(result LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")

add_subdirectory(src)

option(ADD_TESTS "Add tests")
set(ADD_TESTS ON)

if(ADD_TESTS)
    add_subdirectory(tests)
endif()
