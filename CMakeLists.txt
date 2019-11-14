cmake_minimum_required(VERSION 3.5)

project(raycaster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-lX11 -lGL -lpthread -lpng")

add_executable(raycaster main.cpp)
