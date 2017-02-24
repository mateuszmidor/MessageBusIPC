/**
 *   @file: MessageClient.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <pthread.h>
#include "MessageBusIpcCommon.h"
#include "MessageChannel.h"
#include "MessageClient.h"

MessageClient::MessageClient() {
    server_socket_fd = 0;
}

MessageClient::~MessageClient() {
    // close the socket
    if (server_socket_fd)
       close(server_socket_fd);
}

/**
 * @name    connectToMessageHub
 * @brief   Connect to the central message hub; the hub must be already running before you call this function
 * @return  True on successful connection, False otherwise
 */
bool MessageClient::connectToMessageHub() {

   // get a socket filedescriptor
   server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

   // check socket for failure
   if (server_socket_fd == -1) {
      DEBUG_MSG("%s: socket(AF_UNIX, SOCK_STREAM, 0) failed", __FUNCTION__);
      server_socket_fd = 0; // not initialized
      return false;
   }

   DEBUG_MSG("%s: connecting to MessageHub socket: %s...", __FUNCTION__, MESSAGE_HUB_SOCKET_FILENAME);
      sockaddr_un remote;
      remote.sun_family = AF_UNIX;
      strcpy(remote.sun_path, MESSAGE_HUB_SOCKET_FILENAME);
      size_t length = strlen(remote.sun_path) + sizeof(remote.sun_family);
      if (connect(server_socket_fd, (sockaddr*)&remote, length) == -1) {
         DEBUG_MSG("%s: connect failed", __FUNCTION__);
         close(server_socket_fd);
         server_socket_fd = 0; // not initialized
         return false;
      }
   DEBUG_MSG("%s: done.", __FUNCTION__);

   // success
   return true;
}

/**
 * @name    startListen
 * @param   callback Callback function that will handle incoming messages
 * @brief   Start message reception loop, callback is called on every message arrival
 * @note    This is a blocking method. Best called from a separate thread
 */
void MessageClient::startListen(CallbackFunction callback) {
    MessageChannel receiver(server_socket_fd);
    uint32_t message_id = 0;
    uint32_t size = 0;
    char *data = new char[MESSAGE_BUFF_SIZE];

    // reception loop
    while (receiver.receive(message_id, data, size)) {
        callback(message_id, data, size);
    }

    delete[] data;
    DEBUG_MSG("%s: finished listening to incoming messages.", __FUNCTION__);
}

/**
 * @name    send
 * @brief   Send single message to the message hub
 */
bool MessageClient::send(uint32_t message_id, const char *data, uint32_t size) const {
    MessageChannel sender(server_socket_fd);
    return sender.send(message_id, data, size);
}

/**
 * @name    receive
 * @brief   Receive single message from message hub
 * @note    This method should probably be removed as startListen is provided
 */
bool MessageClient::receive(uint32_t &message_id, char *data, uint32_t &size) const {
    MessageChannel receiver(server_socket_fd);
    return receiver.receive(message_id, data, size);
}
