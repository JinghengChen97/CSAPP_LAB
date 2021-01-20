cmake_minimum_required(VERSION 3.17)
project(cachelab_handout)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(cachelab_handout
        csim.c
        trans.c
        cachelab.c
        )
