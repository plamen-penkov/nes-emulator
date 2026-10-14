cmake_minimum_required(VERSION 3.10)
project(NES_Emulator VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(NES_Emulator
    main.cpp
    Bus.cpp
    Cpu6505.cpp
)