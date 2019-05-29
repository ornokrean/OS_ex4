cmake_minimum_required(VERSION 3.12)
project(OS_Ex4)

set(CMAKE_CXX_STANDARD 11)

add_executable(OS_Ex4 FlowExampleSimulation.cpp VirtualMemory.cpp VirtualMemory.h PhysicalMemory.cpp PhysicalMemory.h
        MemoryConstants.h)