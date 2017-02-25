/**
 *   @file: MessageHub.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_

#include "BlockingMessageQueue.h"
#include "SynchronizedChannelList.h"
#include "Server.h"

/**
 * @class   MessageHub
 * @brief   This is the heart of our star-topology MessageBusIPC; all MessageClients connect to MessageHub in order to send-receive messages
 *          and MessageHub forwards messages to the connected MessageClients.
 */
class MessageHub {
public:
    MessageHub();
    virtual ~MessageHub();

    bool runAndForget();

private:

    Server communication_server;
    BlockingMessageQueue message_queue;
    SynchronizedChannelList channel_list;

    bool startMessageRouterThread();
    void startAcceptClients();
    bool handleClientInSeparateThread(MessageChannel &channel);
    static void* handleClientFunc(void* varg);
    static void* routeMessagesFunc(void* varg);

    struct ClientFuncArg {
        ClientFuncArg(MessageChannel c, BlockingMessageQueue &q, SynchronizedChannelList &l) :
                channel(c), message_queue(q), channel_list(l) {
        }
        MessageChannel channel;
        BlockingMessageQueue &message_queue;
        SynchronizedChannelList &channel_list;
    };

    struct RouterFuncArg {
        RouterFuncArg(BlockingMessageQueue &q, SynchronizedChannelList &l) :
                message_queue(q), channel_list(l) {
        }
        BlockingMessageQueue &message_queue;
        SynchronizedChannelList &channel_list;
    };
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_ */
