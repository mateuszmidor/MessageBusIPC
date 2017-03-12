/**
 *   @file: MessageBusIpcCommon.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_

#include <cstdio>
#include <stdint.h>

namespace messagebusipc {


// Message IDs. Start numbering your messages from ID_USER_MESSAGE_BASE
enum MessageBusMessages {
    ID_CLIENT_SAYS_HELLO = 1,                           // sent to the hub and all clients when new client connects, conveys client name
    ID_CLIENT_SAYS_GOODBYE,                             // sent to all clients when client disconnects, conveys client name
    ID_CONNECTED_CLIENT_LIST,                           // broadcasted to all when client connects/disconnects from hub

    ID_USER_MESSAGE_BASE = 1000000                      // user defined messages start from this index
};




// MessageHub listening socket
const char MESSAGE_HUB_SOCKET_FILENAME[] = "/tmp/sdars_ipc_hub";

// Maximum size of single message in bytes
const unsigned MESSAGE_BUFF_SIZE = 1024 * 1024 * 10; // 10MB

// socket file descriptor that is not initialized
const int UNINITIALIZED_SOCKET_FD = -1;

// Message addressed to all connected recipients
const char ALL_CONNECTED_CLIENTS[] = "*";

// Debugging messages routine
#ifndef NDEBUG
# define DEBUG_MSG(fmt, ...) printf(fmt "\n", __VA_ARGS__)
#else
# define DEBUG_MSG(...)
#endif

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_ */
