set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(hub_performancetest
                "source/hub.cpp"
)

target_link_libraries(hub_performancetest MessageBusIpcLib)

target_include_directories(hub_performancetest
                            PUBLIC 
                                "source"
)

add_executable(client_performancetest
                "source/client.cpp"
)

target_link_libraries(client_performancetest MessageBusIpcLib)

target_include_directories(client_performancetest
                            PUBLIC 
                                "source"
)