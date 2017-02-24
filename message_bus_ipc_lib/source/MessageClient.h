/**
 *   @file: MessageClient.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_

#include <stdint.h>


/**
 * @class   MessageClient
 * @brief   This is the message bus client; it connects to the MessageHub and sends and receives messages from it.
 */
class MessageClient {
public:
    typedef void(*CallbackFunction)(uint32_t &id, char *data, uint32_t &size) ;
    MessageClient();
    virtual ~MessageClient();

    bool connectToMessageHub();
    void startListen(CallbackFunction callback);
    bool send(uint32_t id, const char *data, uint32_t size) const;
    bool receive(uint32_t &id, char *data, uint32_t &size) const;

private:
    int server_socket_fd;
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_ */
