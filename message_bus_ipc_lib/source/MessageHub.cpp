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

        // 3. send ID_CLIENT_SAYS_HELLO from new to all connected clients and vice versa
        broadcastClientConnected(channel_list, channel);

        // 4. handle the client in separate thread
        if (!handleClientInSeparateThread(channel))
            DEBUG_MSG("%s: handleClientInSeparateThread failed", __FUNCTION__);
    }
}

/**
 * @name    broadcastClientConnected
 * @brief   Send ID_CLIENT_SAYS_HELLO from new client to all connected clients
 *          and from every connected client to the new client
 */
void MessageHub::broadcastClientConnected(ThreadsafeChannelList &channel_list, MessageChannel &connected) {
    ThreadsafeChannelList::Iterator it  = channel_list.getIterator(); // this is thread sync point
    MessageChannel const * channel;
    const std::string &connected_name = connected.name();
    while ((channel = it.getNext()))
        if (*channel != connected) {
            // new client says hello to existing client
            channel->send(ID_CLIENT_SAYS_HELLO, connected_name.c_str(), connected_name.length() + 1, "");

            // existing client says hello to new client
            const std::string &existing_name = channel->name();
            connected.send(ID_CLIENT_SAYS_HELLO, existing_name.c_str(), existing_name.length() + 1, "");
        }
}

/**
 * @name    broadcastClientDisconnected
 * @brief   Send ID_CLIENT_SAYS_GOODBYE to every connected client
 */
void MessageHub::broadcastClientDisconnected(ThreadsafeChannelList &channel_list, MessageChannel &disconnected) {
    ThreadsafeChannelList::Iterator it  = channel_list.getIterator(); // this is thread sync point
    MessageChannel const * channel;
    const std::string &disconnected_name = disconnected.name();
    while ((channel = it.getNext()))
        if (*channel != disconnected)
            channel->send(ID_CLIENT_SAYS_GOODBYE, disconnected_name.c_str(), disconnected_name.length() + 1, "");
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
    const char *sender_name = channel.name().c_str();

    uint32_t message_id;
    uint32_t size;
    std::string recipient;
    char *data = new char[MESSAGE_BUFF_SIZE];

    while (channel.receive(message_id, data, size, recipient)) {
        const char *message_name = GetMessageName((MessageBusMessage)message_id);
        const char *recipient_name = recipient.c_str();
        DEBUG_MSG("received message %s (%u), %s -> %s, size %d", message_name, message_id, sender_name, recipient_name, size);
        (void)sender_name; (void)message_name; (void)recipient_name; // silent 'unused variable' warning
        arg->message_queue.push(channel, message_id, data, size, recipient);
    }
    DEBUG_MSG("%s: client disconnected: %s", __FUNCTION__, channel.name().c_str());

    broadcastClientDisconnected(arg->channel_list, channel);
    arg->channel_list.removeByValue(channel);
    channel.shutDown(); // make sure the other side knows we are not listening anymore
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
        if (recipient_name == MBUS_ALL_CONNECTED_CLIENTS) {
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
