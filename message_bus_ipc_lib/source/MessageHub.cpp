/**
 *   @file: MessageHub.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <cstdio>
#include <pthread.h>
#include "MessageBusIpcCommon.h"
#include "MessageChannel.h"
#include "MessageHub.h"


MessageHub::MessageHub() {
    server_socket_fd = 0;
}

MessageHub::~MessageHub() {
    cleanupServerSocket();
}

/**
 * @name    runAndForget
 * @brief   Run the MessageHub and forget about it
 * @return  true if successfuly listened for clients, false otherwise
 * @note    This is blocking method. Best called from separate thread
 */
bool MessageHub::runAndForget() {
    // 1. prepare listening server socket
    if (!prepareServerSocket())
        return false;

    // 2. start thread that will route the incoming messages to clients
    if (!startMessageRouterThread())
        return false;

    // 3. start accepting clients
    startAcceptClients();

    return true;
}

/**
 * @name    prepareServerSocket
 * @brief   Initialize listening server socket that will be used to accept clients
 * @return  True on success, False otherwise
 */
bool MessageHub::prepareServerSocket() {

    // delete socket file if such already exists
    unlink(MESSAGE_HUB_SOCKET_FILENAME);

    // get a socket filedescriptor
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // check socket for failure
    if (server_socket_fd == -1) {
        DEBUG_MSG("%s: socket(AF_UNIX, SOCK_STREAM, 0) failed", __FUNCTION__);
        return false;
    }

    DEBUG_MSG("%s: binding listening socket to %s...", __FUNCTION__, MESSAGE_HUB_SOCKET_FILENAME);
        // prepare address struct
        sockaddr_un local;
        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, MESSAGE_HUB_SOCKET_FILENAME);

        // bind socket to address in UNIX domain
        size_t local_length = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(server_socket_fd, (sockaddr*) &local, local_length) == -1) {
            DEBUG_MSG("%s: bind failed with errno %d", __FUNCTION__, errno);
            close(server_socket_fd);
            return false;
        }
    DEBUG_MSG("%s: done.", __FUNCTION__);

    // mark socket as listening socket
    if (listen(server_socket_fd, MAX_AWAITING_CONNECTIONS) == -1) {
        DEBUG_MSG("%s: listen failed with errno %d", __FUNCTION__, errno);
        close(server_socket_fd);
        return false;
    }

    // success
    return true;
}

/**
 * @name    cleanupServerSocket
 * @brief   Get rid of the listening server socket and the backing socket file
 */
void MessageHub::cleanupServerSocket() {
    // close the listening socket
    if (server_socket_fd) {
        close(server_socket_fd);
        server_socket_fd = 0;
    }

    // remove socket file
    unlink(MESSAGE_HUB_SOCKET_FILENAME);
}

/**
 * @name    startMessageRouterThread
 * @brief   Create and run the thread that will route the incoming messages
 * @return  True on successful thread creation, False otherwise
 */
bool MessageHub::startMessageRouterThread() {
    pthread_t thread;
    int return_code;

    RouterFuncArg *arg = new RouterFuncArg(message_queue, channel_list);
    return_code = pthread_create(&thread, NULL, MessageHub::routeMessagesFunc, (void*)arg);
    if (return_code) {
        DEBUG_MSG("%s: pthread_create failed with error code: %d", __FUNCTION__, return_code);
        return false;
    }

    return_code = pthread_detach(thread);
    if (return_code) {
        DEBUG_MSG("%s: pthread_detach failed with error code: %d", __FUNCTION__, return_code);
        return false;
    }

    return true;
}

/**
 * @name    startAcceptClients
 * @brief   Start listening to incoming client connections and handle them in dedicated threads
 */
void MessageHub::startAcceptClients() {
    sockaddr_un remote;
    unsigned remote_length = sizeof(remote);

    while (true) {
        DEBUG_MSG("%s: listening for incoming connection...", __FUNCTION__);

        // 1. accept new client socket
        int client_socket_fd = accept(server_socket_fd, (sockaddr*) &remote, &remote_length); // remote is filled with remote config
        if (client_socket_fd == -1) {
            DEBUG_MSG("%s: accept failed", __FUNCTION__);
            continue;
        }

        channel_list.add(client_socket_fd);

        // 2. handle the client in separate thread
        if (!handleClientInSeparateThread(client_socket_fd)) {
            DEBUG_MSG("%s: handleClient failed", __FUNCTION__);
            continue;
        }
    }
}

/**
 * @name    handleClientInSeparateThread
 * @param   socket_fd Socket file descriptor of the connection that we want to handle
 * @brief   Create a thread and make it handle the new connection
 * @return  True on successful thread creation and run, False otherwise
 */
bool MessageHub::handleClientInSeparateThread(int socket_fd) {
    pthread_t thread;
    int return_code;

    ClientFuncArg *arg = new ClientFuncArg(socket_fd, message_queue, channel_list);
    return_code = pthread_create(&thread, NULL, MessageHub::handleClientFunc, (void*)arg);
    if (return_code) {
        DEBUG_MSG("%s: pthread_create failed with error code: %d", __FUNCTION__, return_code);
        return false;
    }

    return_code = pthread_detach(thread);
    if (return_code) {
        DEBUG_MSG("%s: pthread_detach failed with error code: %d", __FUNCTION__, return_code);
        return false;
    }

    return true;
}

/**
 * @name    handleClientFunc
 * @param   varg Holds ClientFuncArg*
 * @brief   Receive messages from client and push them to the message queue
 * @note    This is run in a dedicated thread
 */
void* MessageHub::handleClientFunc(void* varg) {
    ClientFuncArg *arg = (ClientFuncArg*)varg;

    MessageChannel channel(arg->socket_fd);
    uint32_t message_id;
    uint32_t size;
    char *data = new char[MESSAGE_BUFF_SIZE];

    while (channel.receive(message_id, data, size)) {
        DEBUG_MSG("received message %s", data);
        arg->message_queue.push(channel, message_id, data, size);
    }
    DEBUG_MSG("%s", "Client disconnect.");

    close(arg->socket_fd);
    arg->channel_list.removeByValue(arg->socket_fd);

    delete[] data;
    delete arg;
    return NULL;
}

/**
 * @name    routeMessagesFunc
 * @param   varg Holds RouterFuncArg*
 * @brief   Pop messages from message queue and forward them to connected clients
 * @note    This is run in a dedicated thread
 */
void* MessageHub::routeMessagesFunc(void* varg) {
    RouterFuncArg *arg = (RouterFuncArg*)varg;

    MessageChannel sender;
    MessageChannel const * recipient;
    uint32_t message_id;
    uint32_t size;
    char *data = new char[MESSAGE_BUFF_SIZE];

    while (true) {
        arg->message_queue.pop(sender, message_id, data, size);
        SynchronizedChannelList::Iterator it = arg->channel_list.getIterator();

        while ((recipient = it.getNext()))
            if (*recipient != sender) // dont send the message back to sender
                recipient->send(message_id, data, size);
    }

    delete[] data;
    delete arg;
    return NULL;
}
