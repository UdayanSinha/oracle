cmake_minimum_required(VERSION 3.10)

project(cmake-example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(cmake-example main.c)
install(TARGETS cmake-example DESTINATION bin)
