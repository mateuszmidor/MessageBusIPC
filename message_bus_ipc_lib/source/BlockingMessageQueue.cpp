/**
 *   @file: BlockingQueue.cpp
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#include <string.h>
#include "MessageBusIpcCommon.h"
#include "BlockingMessageQueue.h"

BlockingMessageQueue::BlockingMessageQueue() {
    pthread_mutex_init(&push_pop_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
    pthread_cond_init(&queue_not_full, NULL);

    // for now we only have space for a single message. kind of poor queue :)
    buff = new char[MESSAGE_BUFF_SIZE];
    id = size = 0;
    data_in_buff = false;
}

BlockingMessageQueue::~BlockingMessageQueue() {
    pthread_mutex_destroy(&push_pop_mutex);
    pthread_cond_destroy(&queue_not_empty);
    pthread_cond_destroy(&queue_not_full);
    delete[] buff;
}

/**
 * @name    push
 * @brief   This function copies from data to its internal storage
 */
void BlockingMessageQueue::push(const MessageChannel &sender, uint32_t message_id, const char *data, uint32_t size) {
    pthread_mutex_lock(&push_pop_mutex);

    // wait until there is free space in the queue
    while (data_in_buff)
        pthread_cond_wait(&queue_not_full, &push_pop_mutex);

    this->sender = sender;
    this->id = message_id;
    this->size = size;
    memcpy(this->buff, data, size);

    // signal that the queue now has data in it
    data_in_buff = true;
    pthread_cond_signal (&queue_not_empty);

    pthread_mutex_unlock(&push_pop_mutex);
}

/**
 * @name    pop
 * @brief   This function copies from internal storage to data
 */
void BlockingMessageQueue::pop(MessageChannel &sender, uint32_t &message_id, char *data, uint32_t &size) {
    pthread_mutex_lock(&push_pop_mutex);

    // wait until there is data in the queue
    while (!data_in_buff)
        pthread_cond_wait(&queue_not_empty, &push_pop_mutex);

    sender = this->sender;
    message_id = this->id;
    size = this->size;
    memcpy(data, this->buff, this->size);

    // signal that the queue now has free room
    data_in_buff = false;
    pthread_cond_signal (&queue_not_full);

    pthread_mutex_unlock(&push_pop_mutex);
}
