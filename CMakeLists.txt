cmake_minimum_required(VERSION 2.8.12)

project(viveToPi)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} /home/pi/Documents/simd/src/)

add_executable(viveToPi viveToPi.cpp framebuffer.h camera.h)
target_link_libraries(viveToPi ${OpenCV_LIBS} /home/pi/Documents/simd/build/libSimd.a Threads::Threads)
