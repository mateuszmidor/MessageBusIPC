/**
 *   @file: MessageHub.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include <pthread.h>
#include "MessageBusIpcCommon.h"
#include "MessageChannel.h"
#include "MessageHub.h"

using namespace messagebusipc;


MessageHub::MessageHub() {
}

MessageHub::~MessageHub() {
}

/**
 * @name    runAndForget
 * @brief   Run the MessageHub and forget about it
 * @return  true if successfuly initialized and run, false otherwise
 * @note    This is blocking method. Best called from separate thread
 */
bool MessageHub::runAndForget() {
    // 1. prepare listening server
    if (!server.init())
        return false;

    // 2. start thread that will route the incoming messages to clients
    if (!startMessageRouterThread())
        return false;

    // 3. start accepting clients
    startAcceptClients();

    return true;
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
    return_code = pthread_create(&thread, NULL, MessageHub::routeMessagesFunc, (void*) arg);
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

    while (true) {
        DEBUG_MSG("%s: listening for incoming connection...", __FUNCTION__);

        // 1. accept new communication channel
        MessageChannel channel = server.acceptOne();

        // 2. put it on the list so the router function knows about it
        channel_list.add(channel);

        // 3. handle the client in separate thread
        if (!handleClientInSeparateThread(channel))
            DEBUG_MSG("%s: handleClientInSeparateThread failed", __FUNCTION__);
    }
}

/**
 * @name    handleClientInSeparateThread
 * @param   channel Communication channel of the connection that we want to handle
 * @brief   Create a thread and make it handle the new connection
 * @return  True on successful thread creation and run, False otherwise
 */
bool MessageHub::handleClientInSeparateThread(MessageChannel &channel) {
    pthread_t thread;
    int return_code;

    ClientFuncArg *arg = new ClientFuncArg(channel, message_queue, channel_list);
    return_code = pthread_create(&thread, NULL, MessageHub::handleClientFunc, (void*) arg);
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
    ClientFuncArg *arg = (ClientFuncArg*) varg;

    MessageChannel channel = arg->channel;
    uint32_t message_id;
    uint32_t size;
    char *data = new char[MESSAGE_BUFF_SIZE];

    while (channel.receive(message_id, data, size)) {
        DEBUG_MSG("received message with id %d, size %d", message_id, size);
        arg->message_queue.push(channel, message_id, data, size);
    }
    DEBUG_MSG("%s", "Client disconnect.");

    arg->channel_list.removeByValue(channel);
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
    RouterFuncArg *arg = (RouterFuncArg*) varg;

    MessageChannel sender;
    MessageChannel const * recipient;
    uint32_t message_id;
    uint32_t size;
    char *data = new char[MESSAGE_BUFF_SIZE];

    while (true) {
        arg->message_queue.pop(sender, message_id, data, size);
        ThreadsafeChannelList::Iterator it = arg->channel_list.getIterator();

        while ((recipient = it.getNext()))
            if (*recipient != sender) // dont send the message back to sender
                recipient->send(message_id, data, size);
    }

    delete[] data;
    delete arg;
    return NULL;
}
