# CMake setup
cmake_minimum_required(VERSION 3.10)
project(sessie_0)
set(CMAKE_CXX_STANDARD 98)

# OpenCV libraries
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Output executable
add_executable(sessie_0 main.cpp)
target_link_libraries(sessie_0 ${OpenCV_LIBS})