/**
 *   @file: MessageChannel.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECHANNEL_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECHANNEL_H_

#include <string>
#include <stdint.h>
#include "MessageBusIpcCommon.h"

namespace messagebusipc {

/**
 * @class   MessageChannel
 * @brief   A channel of communication; allows sending and receiving messages over provided socked file descriptor.
 */
class MessageChannel {
public:
    MessageChannel(int socket_fd = UNINITIALIZED_SOCKET_FD);
    ~MessageChannel();
    bool operator==(const MessageChannel& second) const {
        return socket_fd == second.socket_fd;
    }
    bool operator!=(const MessageChannel& second) const {
        return socket_fd != second.socket_fd;
    }

    bool connectToMessageHub();
    bool send(uint32_t id, const char *data, uint32_t size, const char *recipient) const;
    bool receive(uint32_t &id, char *data, uint32_t &size, std::string &recipient, uint32_t max_size = MESSAGE_BUFF_SIZE) const;
    void setName(const std::string &name) { channel_name = name; }
    const std::string &name() const { return channel_name; }

private:
    int socket_fd;
    std::string channel_name;

    bool send_message(uint32_t id, const char *buf, uint32_t size, const char *recipient) const;
    bool send_buffer(const char *buf, uint32_t size) const;

    bool receive_message(uint32_t &id, char* buf, uint32_t &size, std::string &recipient, uint32_t max_size) const;
    bool receive_buffer(char* buf, uint32_t size) const;

    bool isConnected() const;
    void disconnect();

    struct MessageHeader {
        uint32_t id;
        uint32_t size;
        char     recipient_name[20]; // the name could be hashed into a number to improve performance later
    };
};

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECHANNEL_H_ */
