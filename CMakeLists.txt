cmake_minimum_required(VERSION 3.14)
project(linux_serial C)

set(CMAKE_C_STANDARD 11)

add_executable(linux_serial main.c gomi.h)