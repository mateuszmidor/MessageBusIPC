/**
 *   @file: MessageServer.cpp
 *
 *   @date: Feb 24, 2017
 * @author: Mateusz Midor
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <cstdio>

#include "MessageBusIpcCommon.h"
#include "MessageChannel.h"
#include "MessageServer.h"

using namespace messagebusipc;


MessageServer::MessageServer() :
        server_socket_fd(UNINITIALIZED_SOCKET_FD) {
}

MessageServer::~MessageServer() {
    cleanupServerSocket();
}

/**
 * @name    init
 * @brief   Run init before you start accepting clients
 * @return  True on success, False otherwise
 */
bool MessageServer::init() {
    return prepareServerSocket();
}

/**
 * @name    acceptClient
 * @return  MessageChannel that allows communication with accepted client
 * @note    This is blocking function. Best run in dedicated thread
 */
MessageChannel MessageServer::acceptOne() {
    sockaddr_un remote;
    unsigned remote_length = sizeof(remote);
    int client_socket_fd;

    // 1. repeat waiting for client until success
    do {
        client_socket_fd = accept(server_socket_fd, (sockaddr*) &remote, &remote_length);
        if (client_socket_fd == UNINITIALIZED_SOCKET_FD)
            DEBUG_MSG("%s: accept failed, errno %d - %s", __FUNCTION__, errno, strerror(errno));
    } while (client_socket_fd == UNINITIALIZED_SOCKET_FD);

    // 2. client connected, now turn socket into a channel
    MessageChannel channel = prepareChannel(client_socket_fd);
    DEBUG_MSG("%s: client connected: %s", __FUNCTION__, channel.name().c_str());

    return channel;
}

/**
 * @name    prepareChannel
 * @param   socket_fd Socket file descriptor to turn into a channel
 * @return  MessageChannel based on given socket_fd
 */
MessageChannel MessageServer::prepareChannel(int socket_fd) {
    uint32_t message_id; // will be ID_CLIENT_SAYS_HALLO
    uint32_t size;
    std::string name;
    char empty_buffer[1];

    // 1. create a channel
    MessageChannel channel(socket_fd);

    // 2. receive ID_CLIENT_SAYS_HELLO with channel name from MessageClient
    channel.receive(message_id, empty_buffer, size, name);

    // 3. setup channel name
    channel.setName(name);
    return channel;
}
/**
 * @name    prepareServerSocket
 * @brief   Initialize listening server socket that will be used to accept clients
 * @return  True on success, False otherwise
 */
bool MessageServer::prepareServerSocket() {

    // delete socket file if such already exists
    unlink(MESSAGE_HUB_SOCKET_FILENAME);

    // get a socket filedescriptor
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // check socket for failure
    if (server_socket_fd == UNINITIALIZED_SOCKET_FD) {
        DEBUG_MSG("%s: socket(AF_UNIX, SOCK_STREAM, 0) failed, errno %d - %s", __FUNCTION__, errno, strerror(errno));
        return false;
    }

    DEBUG_MSG("%s: binding listening socket to %s...", __FUNCTION__, MESSAGE_HUB_SOCKET_FILENAME);
    // prepare address struct
    sockaddr_un local;
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, MESSAGE_HUB_SOCKET_FILENAME, sizeof(local.sun_path));

    // bind socket to address in UNIX domain
    size_t local_length = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(server_socket_fd, (sockaddr*) &local, local_length) == -1) {
        DEBUG_MSG("%s: bind failed, errno %d - %s", __FUNCTION__, errno, strerror(errno));
        close(server_socket_fd); // cleanup socket filedescriptor
        server_socket_fd = UNINITIALIZED_SOCKET_FD; // status: uninitialized
        return false;
    }
    DEBUG_MSG("%s: done.", __FUNCTION__);

    // mark socket as listening socket
    if (listen(server_socket_fd, MAX_AWAITING_CONNECTIONS) == -1) {
        DEBUG_MSG("%s: listen failed, errno %d - %s", __FUNCTION__, errno, strerror(errno));
        close(server_socket_fd); // cleanup socket filedescriptor
        server_socket_fd = UNINITIALIZED_SOCKET_FD; // status: uninitialized
        return false;
    }

    // success
    return true;
}

/**
 * @name    cleanupServerSocket
 * @brief   Get rid of the listening server socket and the backing socket file
 */
void MessageServer::cleanupServerSocket() {
    // close the listening socket
    if (server_socket_fd != UNINITIALIZED_SOCKET_FD) {
        close(server_socket_fd);
        server_socket_fd = UNINITIALIZED_SOCKET_FD;
    }

    // remove socket file
    unlink(MESSAGE_HUB_SOCKET_FILENAME);
}
