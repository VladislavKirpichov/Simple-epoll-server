cmake_minimum_required(VERSION 3.22)
project(server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)

add_executable(server src/main.c src/http_parser.h src/http_parser.c)
