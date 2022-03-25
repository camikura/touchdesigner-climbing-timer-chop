cmake_minimum_required(VERSION 3.2)
project(ClimbingTimerCHOP)

include_directories(include)

add_library(ClimbingTimerCHOP MODULE src/main.cpp src/serial.cpp)
