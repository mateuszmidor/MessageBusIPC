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


using namespace messagebusipc;


MessageClient::MessageClient() {
    pthread_mutex_init(&send_mutex, NULL);
    message_buffer = new char[MESSAGE_BUFF_SIZE];
    shutting_down = false;
}

MessageClient::~MessageClient() {
    pthread_mutex_destroy(&send_mutex);
    delete[] message_buffer;
}

/**
 * @name    waitForClient
 * @brief   Block current thread until given client becomes available
 */
void MessageClient::waitForClient(const char *client_name) {
    // could be based on synchronization primitives but lets keep things simple :)
    while (!connected_clients.exists(client_name))
        usleep (WAIT_CLIENT_DELAY_USECONDS);
}

/**
 * @name    send
 * @brief   Send single message to the message hub
 * @param   client_name Name of the client this message should be delivered to.
 *                      "*" means all connected clients
 * @note	Thread safe
 */
bool MessageClient::send(uint32_t message_id, const void *data, uint32_t size, const char *client_name) {
    PThreadLockGuard lock(send_mutex); // only one thread can send at a time

    return server_channel.send(message_id, (const char*)data, size, client_name);
}

/**
 * @name    shutDown
 * @brief   Exit the listener loop and close the communication
 */
void MessageClient::shutDown() {
    shutting_down = true;
    server_channel.shutDown();
}

/**
 * @name   tryConnectToMessageHub
 */
bool MessageClient::tryConnectToMessageHub(const char *client_name) {
    // connect to message hub and introduce yourself rightafter
    if (server_channel.connectToMessageHub())
        return server_channel.send(ID_CLIENT_SAYS_HELLO, NULL, 0, client_name);
    else
        return false;
}
