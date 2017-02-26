/**
 *   @file: ThreadsafeMessageQueue.cpp
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#include <string.h>
#include "MessageBusIpcCommon.h"
#include "ThreadsafeMessageQueue.h"

using namespace messagebusipc;

ThreadsafeMessageQueue::ThreadsafeMessageQueue() {
    pthread_mutex_init(&push_pop_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
    pthread_cond_init(&queue_not_full, NULL);

    num_messages = 0;
}

ThreadsafeMessageQueue::~ThreadsafeMessageQueue() {
    pthread_mutex_destroy(&push_pop_mutex);
    pthread_cond_destroy(&queue_not_empty);
    pthread_cond_destroy(&queue_not_full);
}

/**
 * @name    push
 * @brief   This function copies from data to the queue internal storage
 * @note    Thread safe
 */
void ThreadsafeMessageQueue::push(const MessageChannel &sender, uint32_t message_id, const char *data, uint32_t size) {
    pthread_mutex_lock(&push_pop_mutex);

    // wait until there is free space in the queue
    while (num_messages >= MAX_QUEUE_SIZE)
        pthread_cond_wait(&queue_not_full, &push_pop_mutex);

    num_messages++;
    messages[num_messages-1].sender = sender;
    messages[num_messages-1].id = message_id;
    messages[num_messages-1].size = size;
    memcpy(messages[num_messages-1].buff, data, size);

    // signal that the queue now has data in it
    pthread_cond_signal(&queue_not_empty);

    pthread_mutex_unlock(&push_pop_mutex);
}

/**
 * @name    pop
 * @brief   This function copies from the queue internal storage to data
 * @note    Thread safe
 */
void ThreadsafeMessageQueue::pop(MessageChannel &sender, uint32_t &message_id, char *data, uint32_t &size) {
    pthread_mutex_lock(&push_pop_mutex);

    // wait until there is data in the queue
    while (num_messages == 0)
        pthread_cond_wait(&queue_not_empty, &push_pop_mutex);

    sender = messages[num_messages-1].sender;
    message_id = messages[num_messages-1].id;
    size = messages[num_messages-1].size;
    memcpy(data, messages[num_messages-1].buff, messages[num_messages-1].size);
    num_messages--;

    // signal that the queue now has free room
    pthread_cond_signal(&queue_not_full);

    pthread_mutex_unlock(&push_pop_mutex);
}
