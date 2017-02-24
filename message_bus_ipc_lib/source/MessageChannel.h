/**
 *   @file: MessageChannel.h
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECHANNEL_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECHANNEL_H_

#include <stdint.h>
#include "MessageBusIpcCommon.h"

/**
 * @class   MessageChannel
 * @brief   A channel of communication; allows sending and receiving messages over provided socked file descriptor.
 */
class MessageChannel {
public:
    MessageChannel(int socket_fd = -1);
    ~MessageChannel();
    bool operator==(const MessageChannel& second) const { return socket_fd == second.socket_fd; }
    bool operator!=(const MessageChannel& second) const { return socket_fd != second.socket_fd; }

    bool isValid() { return socket_fd > -1; }
    bool connectToMessageHub();
    bool send(uint32_t id, const char *data, uint32_t size) const;
    bool receive(uint32_t &id, char *data, uint32_t &size, uint32_t max_size = MESSAGE_BUFF_SIZE) const;

private:
    int socket_fd;

    bool send_message(uint32_t id, const char *buf, uint32_t size) const;
    bool send_buffer(const char *buf, uint32_t size) const;

    bool receive_message(uint32_t &id, char* buf, uint32_t &size, uint32_t max_size) const;
    bool receive_buffer(char* buf, uint32_t size) const;
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECHANNEL_H_ */
