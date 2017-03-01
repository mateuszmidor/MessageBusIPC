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

/**
 * @name    runAndForget
 * @param   own_thread Should it be run in its own thread(non-blocking run)
 * @brief   Run the MessageHub and forget about it
 * @return  True on succcess, False otherwise
 */
bool MessageHub::runAndForget(bool own_thread) {
    if (own_thread)
        return runInSeparateThread(); // doesn't block
    else
        return (bool)runInCurrentThread(); // does block
}

/**
 * @name    runInSeparateThread
 * @brief   Create a thread and then run the MessageHub in that thread
 * @return  True on success, False otherwise
 */
bool MessageHub::runInSeparateThread() {
    pthread_t thread;
    int return_code;

    return_code = pthread_create(&thread, NULL, MessageHub::runInCurrentThread, NULL);
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
 * @name    runInCurrentThread
 * @param   varg Not used. Only there so this function can be used as pthread_create routine
 */
void* MessageHub::runInCurrentThread(void* varg) {
    MessageHub hub;
    return (void*)hub.run();
}

/**
 * @name    run
 * @brief   Run the MessageHub machinery
 * @return  true if successfully initialized and run, false otherwise
 */
bool MessageHub::run() {
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

        // 3. send list of connected clients to all clients
        broadcastConnectedClientList(channel_list);

        // 4. handle the client in separate thread
        if (!handleClientInSeparateThread(channel))
            DEBUG_MSG("%s: handleClientInSeparateThread failed", __FUNCTION__);
    }
}

void MessageHub::broadcastConnectedClientList(ThreadsafeChannelList &channel_list) {
    ThreadsafeChannelList::Iterator it  = channel_list.getIterator(); // this is thread sync point
    MessageChannel const * recipient;
    std::vector<MessageChannel const *> clients;

    // prepare client list
    while ((recipient = it.getNext()))
        clients.push_back(recipient);

    // for each client create a list of client names excluding itself and send the list to the client
    it.reset();
    while ((recipient = it.getNext())) {
        std::string client_string;
        for (unsigned int i = 0; i < clients.size(); i++)
            if (clients[i] != recipient)
                client_string += clients[i]->name() + ";";
        recipient->send(ID_CONNECTED_CLIENT_LIST, client_string.c_str(), client_string.size() + 1, "");
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
    std::string recipient;
    char *data = new char[MESSAGE_BUFF_SIZE];

    while (channel.receive(message_id, data, size, recipient)) {
        DEBUG_MSG("received message with id %d, size %d", message_id, size);
        arg->message_queue.push(channel, message_id, data, size, recipient);
    }
    DEBUG_MSG("%s", "Client disconnect.");

    arg->channel_list.removeByValue(channel);
    broadcastConnectedClientList(arg->channel_list);
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
    std::string recipient_name;
    char *data = new char[MESSAGE_BUFF_SIZE];

    // route messages forever
    while (true) {
        // get message
        arg->message_queue.pop(sender, message_id, data, size, recipient_name);
        ThreadsafeChannelList::Iterator it = arg->channel_list.getIterator();

        // broadcast
        if (recipient_name == ALL_CONNECTED_CLIENTS) {
            while ((recipient = it.getNext()))
                if (*recipient != sender)
                    recipient->send(message_id, data, size, ""); // dont include recipient_name; no need
        }
        // multicast
        else {
            while ((recipient = it.getNext()))
                if ((*recipient != sender) && (recipient->name() == recipient_name))
                    recipient->send(message_id, data, size, ""); // dont include recipient_name; no need
        }
    }

    delete[] data;
    delete arg;
    return NULL;
}
