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
    pthread_mutex_init(&mutex, NULL);
    buffer = new char[MESSAGE_BUFF_SIZE];
}

MessageClient::~MessageClient() {
    pthread_mutex_destroy(&mutex);
    delete[] buffer;
}

/**
 * @name    initializeAndListen
 * @param   callback Callback function that will handle incoming messages
 * @brief   Initialize connection and start message reception loop, callback is called on every message arrival
 * @note    This is a blocking method. Best called from a separate thread
 */
void MessageClient::initializeAndListen(CallbackFunction callback, bool auto_reconnect) {

    do {
        // 1. if can successfully connect, then listen until connection is broken
        if (tryConnectToMessageHub())
            auto_reconnect &= listenUntilConnectionBroken(callback);

        // 2. sleep a while and maybe reconnect and listen again
        sleep(RECONNECT_DELAY_SECONDS);
    } while (auto_reconnect);

    DEBUG_MSG("%s: finished listening to incoming messages.", __FUNCTION__);
}

/**
 * @name    listenUntilConnectionBroken
 * @return  True if connection broken, False if callback decided to finish listening
 */
bool MessageClient::listenUntilConnectionBroken(CallbackFunction callback) {
    uint32_t message_id = 0;
    uint32_t size = 0;

    while (server_channel.receive(message_id, buffer, size))
        if (callback(message_id, buffer, size) == false) {
            DEBUG_MSG("%s: message callback returns false. Finish reception loop", __FUNCTION__);
            return false;
        }

    // connection broken if we got here
    return true;
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
