cmake_minimum_required(VERSION 3.20)

project(MarvelDatabase C)

set(CMAKE_C_STANDARD 17)

find_package(mongoc CONFIG REQUIRED)

add_executable(MarvelDatabase main.c)

target_link_libraries(MarvelDatabase PRIVATE mongoc::mongoc)