cmake_minimum_required(VERSION 3.12)
project(picomms)
set(CMAKE_CXX_STANDARD 20)
set(COMPILE_DEFINITIONS -O2 -Wall)
add_executable(udp_receive src/udp_receive.cpp)
add_executable(udp_transmit src/udp_transmit.cpp)

