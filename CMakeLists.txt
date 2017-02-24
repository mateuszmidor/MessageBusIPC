cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

#add_definitions(-DNDEBUG)
add_subdirectory("message_bus_ipc_lib")
add_subdirectory("message_bus_ipc_demo")

# Testing required gtest lib
#add_subdirectory("message_bus_ipc_test")