add_library(MessageBusIpcLib
            source/MessageHub.cpp
            source/MessageClient.cpp
            source/MessageChannel.cpp
            source/PThreadLockGuard.cpp
            source/BlockingMessageQueue.cpp
            source/SynchronizedChannelList.cpp
)

target_include_directories(MessageBusIpcLib
                            PUBLIC 
                                "source"
)