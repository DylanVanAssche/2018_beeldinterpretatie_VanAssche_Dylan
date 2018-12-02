# CMake setup
cmake_minimum_required(VERSION 3.9)
project(project)
set(CMAKE_CXX_STANDARD 98)

# OpenCV libraries
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Output executable
add_executable(project main.cpp notes.h)
target_link_libraries(project ${OpenCV_LIBS})
