/**
 *   @file: MessageClient.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_

#include <stdint.h>
#include "PThreadLockGuard.h"
#include "ThreadsafeClientList.h"
#include "MessageChannel.h"

namespace messagebusipc {

/**
 * @class   MessageClient
 * @brief   This is the message bus client; it connects to the MessageHub and sends and receives messages from it.
 */
class MessageClient {
public:
    MessageClient();
    virtual ~MessageClient();

    /**
     * @name    initializeAndListenMemberFunc
     * @param   obj Pointer to class object
     * @param   memfun Pointer to class member function that will handle incoming messages
     * @brief   Initialize connection and start message reception loop, callback is called on every message arrival
     * @note    This is a blocking method. Best called from a separate thread
     * @note    memfun: bool (*memfun)(uint32_t &id, char *data, uint32_t &size);
     */
    template <class ObjPtr, class MemFuncPtr>
    void initializeAndListenMemberFunc(ObjPtr obj, MemFuncPtr memfun, const char *client_name, bool auto_reconnect = true) {
        MemberCallback<ObjPtr, MemFuncPtr> mc(obj, memfun);
        initializeAndListen(mc, client_name, auto_reconnect);
    }

    /**
     * @name    initializeAndListen
     * @param   callback Callback function that will handle incoming messages
     * @brief   Initialize connection and start message reception loop, callback is called on every message arrival
     * @note    This is a blocking method. Best called from a separate thread
     * @note    memfun: bool (*memfun)(uint32_t &id, char *data, uint32_t &size);
     */
    template <class Callback>
    void initializeAndListen(Callback callback, const char *client_name, bool auto_reconnect = true) {
        do {
            // 1. if can successfully connect, then listen until connection is broken
            if (tryConnectToMessageHub(client_name))
                auto_reconnect &= listenUntilConnectionBroken(callback);

            // 2. we got here so connection broken; clear available client list
            connected_clients.update("");

            // 3. sleep a while and maybe reconnect and listen again
            sleep(RECONNECT_DELAY_SECONDS);
        } while (auto_reconnect);

        DEBUG_MSG("%s: finished listening to incoming messages.", __FUNCTION__);
    }

    void waitForClient(const char *client_name);
    bool send(uint32_t id, const char *data, uint32_t size, const char *client_name);

private:
    char *message_buffer;
    MessageChannel server_channel;
    pthread_mutex_t send_mutex;
    ThreadsafeClientList connected_clients;
    static const int RECONNECT_DELAY_SECONDS = 3;
    static const int WAIT_CLIENT_DELAY_MSECONDS = 100;

    bool tryConnectToMessageHub(const char *client_name);

    /**
     * @name    listenUntilConnectionBroken
     * @return  True if connection broken, False if callback decided to finish listening
     */
    template <class Callback>
    bool listenUntilConnectionBroken(Callback callback) {
        uint32_t message_id = 0;
        uint32_t size = 0;
        std::string recipient; // discard this as we know who we are

        while (server_channel.receive(message_id, message_buffer, size, recipient))
            if (message_id == ID_CONNECTED_CLIENT_LIST) {
                DEBUG_MSG("%s: update connected clients: [%s]", __FUNCTION__, message_buffer);
                connected_clients.update(message_buffer);
            }
            else if (callback(message_id, message_buffer, size) == false) {
                DEBUG_MSG("%s: message callback returns false. Finish reception loop", __FUNCTION__);
                return false;
            }

        // connection broken if we got here
        return true;
    }


    // Functor for calling object member function
    template <class T, class F>
    struct MemberCallback {
        T t;
        F f;
        MemberCallback(T t, F f) : t(t), f(f) {};
        bool operator()(uint32_t &id, char *data, uint32_t &size) {
            return ((t)->*(f))(id, data, size);
        }

    };
};

}; // namepace messagebusipc



#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_ */
