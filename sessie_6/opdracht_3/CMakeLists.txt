# CMake setup
cmake_minimum_required(VERSION 3.9)
project(sessie_6-3)
set(CMAKE_CXX_STANDARD 98)

# OpenCV libraries
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Output executable
add_executable(sessie_6-3 main.cpp people.h)
target_link_libraries(sessie_6-3 ${OpenCV_LIBS})
