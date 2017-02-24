/**
 *   @file: Server.h
 *
 *   @date: Feb 24, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_SERVER_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_SERVER_H_

#include "MessageChannel.h"
/**
 * @class   Server
 * @brief   Server awaits incoming connections and return them as MessageChannels
 */
class Server {
public:
    Server();
    virtual ~Server();

    bool init();
    MessageChannel acceptOne();

private:
    const static int MAX_AWAITING_CONNECTIONS = 10;
    int server_socket_fd;

    bool prepareServerSocket();
    void cleanupServerSocket();
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_SERVER_H_ */
