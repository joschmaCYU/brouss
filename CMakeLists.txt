cmake_minimum_required(VERSION 3.10)
project(brouss)

# FORCE DEBUG MODE
# This ensures variables are visible in the debugger
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)

add_executable(brouss src/main.cpp)
