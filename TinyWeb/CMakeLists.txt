cmake_minimum_required(VERSION 3.17)
project(TinyWeb)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinyWeb tinyweb.c  csapp.c)

target_link_libraries(${PROJECT_NAME} pthread)