/**
 *   @file: MessageBusIpcCommon.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_

// MessageHub listening socket
const char MESSAGE_HUB_SOCKET_FILENAME[] = "/tmp/sdars_ipc_hub";

// Maximum size of single message in bytes
const unsigned MESSAGE_BUFF_SIZE = 1024 * 1024 * 1; // 1MB

// socket file descriptor that is not initialized
const int UNINITIALIZED_SOCKET_FD = -1;

// Debugging messages routine
#ifndef NDEBUG
#include <cstdio>
# define DEBUG_MSG(fmt, ...) printf(fmt "\n", __VA_ARGS__)
#else
# define DEBUG_MSG(...)
#endif

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEBUSIPCCOMMON_H_ */
