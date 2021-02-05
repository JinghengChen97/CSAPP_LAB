cmake_minimum_required(VERSION 3.17)
project(malloc_lab)

set (CMAKE_C_FLAGS "-m32")
set(CMAKE_C_COMPILER gcc)

add_executable(${PROJECT_NAME} mdriver.c memlib.c mm.c fsecs.c fcyc.c ftimer.c clock.c)
add_executable(${PROJECT_NAME}_tester my_tester.c memlib.c mm.c )


target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME}_tester pthread)