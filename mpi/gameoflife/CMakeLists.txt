cmake_minimum_required(VERSION 3.1)
project(Game_Of_Life)

find_package(MPI REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(gol main.cpp)

target_link_libraries(gol ${MPI_LIBRARIES} ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
