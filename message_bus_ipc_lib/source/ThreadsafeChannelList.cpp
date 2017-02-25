/**
 *   @file: ThreadsafeChannelList.cpp
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#include "ThreadsafeChannelList.h"

#include <algorithm>
#include "MessageBusIpcCommon.h"
#include "PThreadLockGuard.h"

/**
 * Iterator Constructor.
 * @param   channels MessageChannel list to iterate over
 * @param   mutex ThreadsafeChannelList add/remove/iterate synchronization mutex
 * @brief   channellist_lock locks the mutex, so add/remove operations of ThreadsafeChannelList are blocked
 */
ThreadsafeChannelList::Iterator::Iterator(std::vector<MessageChannel> &channels, pthread_mutex_t &mutex) :
        channels(channels), current_position(0), channellist_lock(mutex) {
}

/**
 * Iterator Destructor.
 */
ThreadsafeChannelList::Iterator::~Iterator() {
    // channellist_lock gets destroyed and unlocks the mutex, so add/remove operations of ThreadsafeChannelList get unlocked
}

/**
 * @name    getNext
 * @brief   Get MessageChannel pointer and advance the iterator
 * @return  MessageChannel pointer if iterating not done yet, NULL otherwise
 */
MessageChannel const* ThreadsafeChannelList::Iterator::getNext() {
    if (current_position == channels.size())
        return NULL;

    return &channels[current_position++];
}

/**
 * ThreadsafeChannelList Constructor.
 * @brief   Initialize the add/remove/iterate mutex
 */
ThreadsafeChannelList::ThreadsafeChannelList() {
    pthread_mutex_init(&channels_mutex, NULL);
}

/**
 * ThreadsafeChannelList Destructor.
 * @brief   Get rid of the add/remove/iterate mutex
 */
ThreadsafeChannelList::~ThreadsafeChannelList() {
    pthread_mutex_destroy(&channels_mutex);
}

/**
 * @name    add
 * @param   channel
 * @note    Thread safe add/remove/iterate
 */
void ThreadsafeChannelList::add(MessageChannel &channel) {
    PThreadLockGuard lock(channels_mutex);

    channels.push_back(channel);
    DEBUG_MSG("%s: num channels: %d", __FUNCTION__, (int )channels.size());
}

/**
 * @name    remove
 * @param   index Index of item on the list to remove
 * @note    Thread safe add/remove/iterate
 */
void ThreadsafeChannelList::remove(unsigned int index) {
    PThreadLockGuard lock(channels_mutex);

    if (index >= channels.size())
        return;

    channels.erase(channels.begin() + index);
    DEBUG_MSG("%s: num channels: %d", __FUNCTION__, (int )channels.size());
}

/**
 * @name    removeByValue
 * @param   channel MessageChannel to remove
 * @note    Thread safe add/remove/iterate
 */
void ThreadsafeChannelList::removeByValue(MessageChannel &channel) {
    PThreadLockGuard lock(channels_mutex);

    channels.erase(std::remove(channels.begin(), channels.end(), channel), channels.end());
    DEBUG_MSG("%s: num channels: %d", __FUNCTION__, (int )channels.size());
}

/**
 * @name    getIterator
 * @return  MessageChannel iterator
 * @note    Thread safe add/remove/iterate
 */
ThreadsafeChannelList::Iterator ThreadsafeChannelList::getIterator() {
    return Iterator(channels, channels_mutex);
}
