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

#define ENUM_DEFINE1_OPERATOR(name)             name,
#define ENUM_DEFINE2_OPERATOR(name, val)        name = val,
#define ENUM_NAME1_OPERATOR(name)               #name,
#define ENUM_NAME2_OPERATOR(name, val)          #name,
#define ENUM_ENUMVAL1_OPERATOR(name)            name,
#define ENUM_ENUMVAL2_OPERATOR(name, val)       name,
#define ENUM_COMMENT_OPERATOR(...)

#define MBIPC_MESSAGES(OP1, OP2, COM)\
   OP2(ID_USER_MESSAGE_BASE, 0) COM("user defined messages start from this index") \
   COM() \
   COM("IPC internal messages") \
   OP2(ID_CLIENT_SAYS_HELLO, 1000000) COM("sent to the hub and all clients when new client connects, conveys client name") \
   OP1(ID_CLIENT_SAYS_GOODBYE) COM("sent to all clients when client disconnects, conveys client name") \

// here enum definition becomes real
enum MessageBusMessage { MBIPC_MESSAGES(ENUM_DEFINE1_OPERATOR, ENUM_DEFINE2_OPERATOR, ENUM_COMMENT_OPERATOR) };

// use this function to get enum string representation
const char * GetMessageName(MessageBusMessage Msg);



// MessageHub listening socket
const char MESSAGE_HUB_SOCKET_FILENAME[] = "/tmp/ipc_hub";

// Maximum size of single message in bytes
const unsigned MESSAGE_BUFF_SIZE = 1024 * 1024 * 10; // 10MB

// socket file descriptor that is not initialized
const int UNINITIALIZED_SOCKET_FD = -1;

// Message addressed to all connected recipients
const char MBUS_ALL_CONNECTED_CLIENTS[] = "*";

// Debugging messages routine
#ifndef NDEBUG
#define DEBUG_MSG(fmt, ...) printf("[IPC] " fmt "\n", __VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_ */
