cmake_minimum_required(VERSION 3.0)

project(UP_DOWN_SHOW_THREADS_SERVER)

add_executable(exe up_down_show_var_server.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
