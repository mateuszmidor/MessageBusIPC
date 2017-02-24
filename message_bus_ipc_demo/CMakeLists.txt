set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(hub
                "source/hub.cpp"
)

target_link_libraries(hub MessageBusIpcLib)

target_include_directories(hub
                            PUBLIC 
                                "source"
)

add_executable(client
                "source/client.cpp"
)

target_link_libraries(client MessageBusIpcLib)

target_include_directories(client
                            PUBLIC 
                                "source"
)