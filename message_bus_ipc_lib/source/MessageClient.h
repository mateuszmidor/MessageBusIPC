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

/**
 * @class   MessageClient
 * @brief   This is the message bus client; it connects to the MessageHub and sends and receives messages from it.
 */
class MessageClient {
public:
    typedef bool(*CallbackFunction)(uint32_t &id, char *data, uint32_t &size) ;

    MessageClient();
    virtual ~MessageClient();

    void initializeAndListen(CallbackFunction callback, bool auto_reconnect = true);
    bool send(uint32_t id, const char *data, uint32_t size);

private:
    char *buffer;
    MessageChannel server_channel;
    pthread_mutex_t mutex;
	static const int RECONNECT_DELAY_SECONDS = 3;

    bool tryConnectToMessageHub();
	bool listenUntilConnectionBroken(CallbackFunction callback);
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_MESSAGECLIENT_H_ */
