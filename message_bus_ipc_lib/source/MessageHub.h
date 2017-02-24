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

/**
 * @class   MessageHub
 * @brief   This is the heart of our star-topology MessageBusIPC; all MessageClients connect to MessageHub in order to send-receive messages
 *          and MessageHub forwards messages to the connected MessageClients.
 */
class MessageHub {

    struct ClientFuncArg {
        ClientFuncArg(int fd, BlockingMessageQueue &q, SynchronizedChannelList &l) : socket_fd(fd), message_queue(q), channel_list(l)  {}
        int socket_fd;
        BlockingMessageQueue &message_queue;
        SynchronizedChannelList &channel_list;
    };

    struct RouterFuncArg {
        RouterFuncArg(BlockingMessageQueue &q, SynchronizedChannelList &l) : message_queue(q), channel_list(l) {}
        BlockingMessageQueue &message_queue;
        SynchronizedChannelList &channel_list;
    };

public:
    MessageHub();
    virtual ~MessageHub();

    bool runAndForget();

private:
    const static int MAX_AWAITING_CONNECTIONS = 10;

    int server_socket_fd;
    BlockingMessageQueue message_queue;
    SynchronizedChannelList channel_list;

    bool prepareServerSocket();
    void cleanupServerSocket();
    bool startMessageRouterThread();
    void startAcceptClients();
    bool handleClientInSeparateThread(int socket_fd);
    static void* handleClientFunc(void* varg);
    static void* routeMessagesFunc(void* varg);
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGEHUB_H_ */
