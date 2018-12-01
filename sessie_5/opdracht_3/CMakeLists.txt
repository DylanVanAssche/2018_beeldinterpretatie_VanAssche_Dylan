# CMake setup
cmake_minimum_required(VERSION 3.9)
project(sessie_5-3)
set(CMAKE_CXX_STANDARD 98)

# OpenCV libraries
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Output executable
add_executable(sessie_5-3 main.cpp)
target_link_libraries(sessie_5-3 ${OpenCV_LIBS})
