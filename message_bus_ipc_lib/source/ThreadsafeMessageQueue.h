/**
 *   @file: ThreadsafeMessageQueue.h
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFEMESSAGEQUEUE_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFEMESSAGEQUEUE_H_

#include <pthread.h>
#include <stdint.h>
#include "MessageChannel.h"

/**
 * @class   ThreadsafeMessageQueue
 * @brief   Thread-safe message queue for Producer-Consumer processing scheme of messages.
 */
class ThreadsafeMessageQueue {
public:
    ThreadsafeMessageQueue();
    virtual ~ThreadsafeMessageQueue();

    void push(const MessageChannel &sender, uint32_t id, const char *data, uint32_t size);
    void pop(MessageChannel &sender, uint32_t &id, char *data, uint32_t &size);

private:
    pthread_mutex_t push_pop_mutex;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
    volatile bool data_in_buff;

    // message data
    char *buff;
    uint32_t id, size;
    MessageChannel sender;
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFEMESSAGEQUEUE_H_ */