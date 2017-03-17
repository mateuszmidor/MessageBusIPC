add_library(MessageBusIpcLib
            source/MessageServer.cpp
            source/MessageHub.cpp
            source/MessageClient.cpp
            source/MessageChannel.cpp
            source/MessageBusIpcCommon.cpp
            source/PThreadLockGuard.cpp
            source/ThreadsafeMessageQueue.cpp
            source/ThreadsafeChannelList.cpp
            source/ThreadsafeClientList.cpp
)

target_include_directories(MessageBusIpcLib
                            PUBLIC
                                "source"
)

find_package(Threads REQUIRED)
target_link_libraries(MessageBusIpcLib Threads::Threads)