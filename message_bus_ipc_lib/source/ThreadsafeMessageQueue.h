/**
 *   @file: ThreadsafeMessageQueue.h
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFEMESSAGEQUEUE_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFEMESSAGEQUEUE_H_

#include <string>
#include <pthread.h>
#include <stdint.h>
#include "MessageChannel.h"
#include "MessageBusIpcCommon.h"
namespace messagebusipc {


/**
 * @class   ThreadsafeMessageQueue
 * @brief   Thread-safe message queue for Producer-Consumer processing scheme of messages.
 */
class ThreadsafeMessageQueue {
public:
    ThreadsafeMessageQueue();
    virtual ~ThreadsafeMessageQueue();

    void push(const MessageChannel &sender, uint32_t id, const char *data, uint32_t size, const std::string &recipient);
    void pop(MessageChannel &sender, uint32_t &id, char *data, uint32_t &size, std::string &recipient);

private:
    pthread_mutex_t push_pop_mutex;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;

    // message data queue
    struct Message {
        Message() { buff = new char[MESSAGE_BUFF_SIZE]; id = 0; size = 0; }
        ~Message() { delete[] buff; }
        char *buff;
        uint32_t id, size;
        MessageChannel sender;
        std::string recipient;
    };
    static const int MAX_QUEUE_SIZE = 10;
    Message messages[MAX_QUEUE_SIZE];
    int reader_pos, writer_pos;


};

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFEMESSAGEQUEUE_H_ */
