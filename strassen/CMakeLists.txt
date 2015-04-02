cmake_minimum_required(VERSION 3.1)
project(strassen)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

add_executable(sequential sequential.cc matrix_sequential.h matrix.h)

add_executable(parallel1 parallel1.cc matrix_parallel1.h matrix.h)

add_executable(comparator comparator.cc matrix_sequential.h matrix_parallel1.h matrix.h)