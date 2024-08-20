cmake_minimum_required(VERSION 3.12)
project(picomms)
set(CMAKE_CXX_STANDARD 20)
set(COMPILE_DEFINITIONS -O2 -Wall)
add_executable(udp_chat src/udp_chat.cpp)

