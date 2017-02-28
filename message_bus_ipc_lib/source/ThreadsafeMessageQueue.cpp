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

    reader_pos = 0;
    writer_pos = 0;
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
void ThreadsafeMessageQueue::push(const MessageChannel &sender, uint32_t message_id, const char *data, uint32_t size, const std::string &recipient) {
    pthread_mutex_lock(&push_pop_mutex);

    // wait until there is free space in the queue
    while (((writer_pos+1) % MAX_QUEUE_SIZE) == reader_pos)
        pthread_cond_wait(&queue_not_full, &push_pop_mutex);

    Message &m = messages[writer_pos];
    m.sender = sender;
    m.id = message_id;
    m.size = size;
    m.recipient = recipient;
    memcpy(m.buff, data, size);
    writer_pos = (writer_pos + 1) % MAX_QUEUE_SIZE; // advance the writer

    // signal that the queue now has data in it
    pthread_cond_signal(&queue_not_empty);

    pthread_mutex_unlock(&push_pop_mutex);
}

/**
 * @name    pop
 * @brief   This function copies from the queue internal storage to data
 * @note    Thread safe
 */
void ThreadsafeMessageQueue::pop(MessageChannel &sender, uint32_t &message_id, char *data, uint32_t &size, std::string &recipient) {
    pthread_mutex_lock(&push_pop_mutex);

    // wait until there is data in the queue
    while (reader_pos == writer_pos)
        pthread_cond_wait(&queue_not_empty, &push_pop_mutex);

    Message &m = messages[reader_pos];
    sender = m.sender;
    message_id = m.id;
    size = m.size;
    recipient = m.recipient;
    memcpy(data, m.buff, m.size);
    reader_pos = (reader_pos + 1) % MAX_QUEUE_SIZE; // advance the reader

    // signal that the queue now has free room
    pthread_cond_signal(&queue_not_full);

    pthread_mutex_unlock(&push_pop_mutex);
}
