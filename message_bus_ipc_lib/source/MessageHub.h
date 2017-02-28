/**
 *   @file: MessageHub.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_

#include "MessageServer.h"
#include "ThreadsafeChannelList.h"
#include "ThreadsafeMessageQueue.h"

namespace messagebusipc {

/**
 * @class   MessageHub
 * @brief   This is the heart of our star-topology MessageBusIPC; all MessageClients connect to MessageHub in order to send-receive messages
 *          and MessageHub forwards messages to the connected MessageClients.
 */
class MessageHub {
public:
    static bool runAndForget(bool own_thread = false);

private:

    MessageServer server;
    ThreadsafeMessageQueue message_queue;
    ThreadsafeChannelList channel_list;

    bool run();
    bool startMessageRouterThread();
    void startAcceptClients();
    bool handleClientInSeparateThread(MessageChannel &channel);
    static bool runInSeparateThread();
    static void* runInCurrentThread(void* varg = NULL);
    static void* handleClientFunc(void* varg);
    static void* routeMessagesFunc(void* varg);

    struct ClientFuncArg {
        ClientFuncArg(MessageChannel c, ThreadsafeMessageQueue &q, ThreadsafeChannelList &l) :
                channel(c), message_queue(q), channel_list(l) {
        }
        MessageChannel channel;
        ThreadsafeMessageQueue &message_queue;
        ThreadsafeChannelList &channel_list;
    };

    struct RouterFuncArg {
        RouterFuncArg(ThreadsafeMessageQueue &q, ThreadsafeChannelList &l) :
                message_queue(q), channel_list(l) {
        }
        ThreadsafeMessageQueue &message_queue;
        ThreadsafeChannelList &channel_list;
    };
};

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_ */
