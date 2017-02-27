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
bool MessageClient::send(uint32_t message_id, const char *data, uint32_t size) {
    PThreadLockGuard lock(mutex); // only one thread can send at a time

    return server_channel.send(message_id, data, size);
}

/**
 * @name   tryConnectToMessageHub
 */
bool MessageClient::tryConnectToMessageHub() {
    return server_channel.connectToMessageHub();
}
