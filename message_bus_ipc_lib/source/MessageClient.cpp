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
    pthread_mutex_init(&mutex, NULL);
    buffer = new char[MESSAGE_BUFF_SIZE];
}

MessageClient::~MessageClient() {
    pthread_mutex_destroy(&mutex);
    delete[] buffer;
}

/**
 * @name    send
 * @brief   Send single message to the message hub
 * @note	Thread safe
 */
bool MessageClient::send(uint32_t message_id, const char *data, uint32_t size, const char *recipient_name) {
    PThreadLockGuard lock(mutex); // only one thread can send at a time

    return server_channel.send(message_id, data, size, recipient_name);
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
