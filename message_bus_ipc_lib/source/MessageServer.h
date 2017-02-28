/**
 *   @file: MessageServer.h
 *
 *   @date: Feb 24, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGESERVER_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGESERVER_H_

#include "MessageChannel.h"

namespace messagebusipc {

/**
 * @class   MessageServer
 * @brief   Server awaits incoming connections and return them as MessageChannels
 */
class MessageServer {
public:
    MessageServer();
    virtual ~MessageServer();

    bool init();
    MessageChannel acceptOne();

private:
    const static int MAX_AWAITING_CONNECTIONS = 10;
    int server_socket_fd;

    MessageChannel prepareChannel(int socket_fd);
    bool prepareServerSocket();
    void cleanupServerSocket();
};

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGESERVER_H_ */
