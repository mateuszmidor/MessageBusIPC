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
    void initializeAndListenMemberFunc(ObjPtr obj, MemFuncPtr memfun, bool auto_reconnect = true) {
        MemberCallback<ObjPtr, MemFuncPtr> mc(obj, memfun);
        initializeAndListen(mc, auto_reconnect);
    }

    /**
     * @name    initializeAndListen
     * @param   callback Callback function that will handle incoming messages
     * @brief   Initialize connection and start message reception loop, callback is called on every message arrival
     * @note    This is a blocking method. Best called from a separate thread
     * @note    memfun: bool (*memfun)(uint32_t &id, char *data, uint32_t &size);
     */
    template <class Callback>
    void initializeAndListen(Callback callback, bool auto_reconnect = true) {
        do {
            // 1. if can successfully connect, then listen until connection is broken
            if (tryConnectToMessageHub())
                auto_reconnect &= listenUntilConnectionBroken(callback);

            // 2. sleep a while and maybe reconnect and listen again
            sleep(RECONNECT_DELAY_SECONDS);
        } while (auto_reconnect);

        DEBUG_MSG("%s: finished listening to incoming messages.", __FUNCTION__);
    }

    bool send(uint32_t id, const char *data, uint32_t size);

private:
    char *buffer;
    MessageChannel server_channel;
    pthread_mutex_t mutex;
    static const int RECONNECT_DELAY_SECONDS = 3;

    bool tryConnectToMessageHub();


    /**
     * @name    listenUntilConnectionBroken
     * @return  True if connection broken, False if callback decided to finish listening
     */
    template <class Callback>
    bool listenUntilConnectionBroken(Callback callback) {
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
