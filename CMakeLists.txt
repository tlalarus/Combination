cmake_minimum_required(VERSION 3.23)
project(combtest)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIR})

add_executable(combtest main.cpp)

target_link_libraries(
        ${PROJECT_NAME}
        ${OpenCV_LIBRARIES}
)