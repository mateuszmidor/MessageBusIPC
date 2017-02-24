/**
 *   @file: MessageChannel.cpp
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
#include <cassert>
#include "MessageBusIpcCommon.h"
#include "MessageChannel.h"

MessageChannel::MessageChannel(int socket_fd) :
    socket_fd(socket_fd) {
}

MessageChannel::~MessageChannel() {
    //TODO: make channel a ref-counted resource and close the socket when no references
//    if (socket_fd > -1)
//        close(socket_fd);
}

MessageChannel::MessageChannel(const MessageChannel &second) {
    socket_fd = second.socket_fd;//dup(second.socket_fd);
    DEBUG_MSG("socket_fd %d", socket_fd);
}

MessageChannel& MessageChannel::operator=(const MessageChannel &second) {
    socket_fd = second.socket_fd;//dup(second.socket_fd);
    DEBUG_MSG("socket_fd %d", socket_fd);
    return *this;
}

/**
 * @name    connectToMessageHub
 * @brief   Connect to the central message hub; the hub must be already running before you call this function
 * @return  True on successful connection, False otherwise
 */
bool MessageChannel::connectToMessageHub() {

   // get a socket filedescriptor
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

   // check socket for failure
   if (socket_fd == -1) {
      DEBUG_MSG("%s: socket(AF_UNIX, SOCK_STREAM, 0) failed", __FUNCTION__);
      return false;
   }

   DEBUG_MSG("%s: connecting to MessageHub socket: %s...", __FUNCTION__, MESSAGE_HUB_SOCKET_FILENAME);
      sockaddr_un remote;
      remote.sun_family = AF_UNIX;
      strcpy(remote.sun_path, MESSAGE_HUB_SOCKET_FILENAME);
      size_t length = strlen(remote.sun_path) + sizeof(remote.sun_family);
      if (connect(socket_fd, (sockaddr*)&remote, length) == -1) {
         DEBUG_MSG("%s: connect failed", __FUNCTION__);
         close(socket_fd);
         socket_fd = -1; // not initialized
         return false;
      }
   DEBUG_MSG("%s: done.", __FUNCTION__);

   // success
   return true;
}

/**
 * @name    send
 * @brief   Send a message over a socket
 * @param   id Message ID
 * @param   data Message payload
 * @param   size Message payload size in bytes
 * @return  True if send was successful, False otherwise
 */
bool MessageChannel::send(uint32_t message_id, const char *data, uint32_t size) const {

//   std::lock_guard<std::mutex> guard(mtx);
    if (!socket_fd) {
        DEBUG_MSG("%s: socket_fd not initialized", __FUNCTION__);
        return false;
    }

    // send the message
    if (!send_message(message_id, data, size)) {
        DEBUG_MSG("%s: send_message failed", __FUNCTION__);
        if (errno == EPIPE)
            DEBUG_MSG("%s: errno: EPIPE (connection broken)", __FUNCTION__);
        return false;
    }

    return true;
}

/**
 * @name    send_message
 * @note    Implementation detail
 */
bool MessageChannel::send_message(uint32_t id, const char *buf, uint32_t size) const {
    if (!send_buffer(reinterpret_cast<char*>(&id), sizeof(id)))
        return false;

    if (!send_buffer(reinterpret_cast<char*>(&size), sizeof(size)))
        return false;

    if (!send_buffer(buf, size))
        return false;

    return true;
}

/**
 * @name    send_buffer
 * @note    Implementation detail
 */
bool MessageChannel::send_buffer(const char *buf, uint32_t size) const {
    uint32_t bytes_left = size;
    int num_bytes_sent;
    while ((bytes_left > 0) && ((num_bytes_sent = ::send(socket_fd, buf, bytes_left, MSG_NOSIGNAL)) > 0)) {
        bytes_left -= num_bytes_sent;
        buf += num_bytes_sent;
    }

    return (bytes_left == 0); // success if all bytes sent
}

/**
 * @name    receive
 * @param   message_id ID of received message
 * @param   buf Message payload buffer
 * @param   size Payload size in bytes
 * @param   max_size Maximum number of bytes that can fit into the buffer
 * @return  True on success, False on error
 */
bool MessageChannel::receive(uint32_t &message_id, char* buf, uint32_t &size, uint32_t max_size) const {

    if (!socket_fd) {
        DEBUG_MSG("%s: socket_fd not initialized", __FUNCTION__);
        return false;
    }

    // send the message
    if (!receive_message(message_id, buf, size, max_size)) {
        switch (errno) {
        case 0:
            return false;
        case EPIPE:
            DEBUG_MSG("%s: receive_message failed, errno: EPIPE (connection broken)", __FUNCTION__);
            return false;
        default:
            DEBUG_MSG("%s: receive_message failed, errno: %d", __FUNCTION__, errno);
            return false;
        }
    }

    return true;
}

/**
 * @name    receive_message
 * @param   max_size    Maximum number of bytes that can fit into the buffer
 * @note    Implementation detail
 */
bool MessageChannel::receive_message(uint32_t &id, char* buf, uint32_t &size, uint32_t max_size) const {
    if (!receive_buffer(reinterpret_cast<char*>(&id), sizeof(id)))
        return false;

    if (!receive_buffer(reinterpret_cast<char*>(&size), sizeof(size)))
        return false;

    assert(size <= max_size && "Received message size exceeds reception buffer size!");

    if (!receive_buffer(buf, size))
        return false;

    return true;
}

/**
 * @name    receive_buffer
 * @note    Implementation detail
 */
bool MessageChannel::receive_buffer(char* buf, uint32_t size) const {
    uint32_t num_bytes_left = size;
    int num_bytes_received;
    while ((num_bytes_left > 0) && ((num_bytes_received = recv(socket_fd, buf, num_bytes_left, 0)) > 0)) {
        num_bytes_left -= num_bytes_received;
        buf += num_bytes_received;
    }

    return (num_bytes_left == 0);
}
