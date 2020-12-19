cmake_minimum_required(VERSION 3.17)
project(malloc_lab)

set(CMAKE_C_COMPILER gcc)

add_executable(${PROJECT_NAME} mdriver.c memlib.c mm.c fsecs.c fcyc.c ftimer.c clock.c)
target_link_libraries(${PROJECT_NAME} pthread)