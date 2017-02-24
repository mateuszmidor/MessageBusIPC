/**
 *   @file: SynchronizedChannelList.cpp
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "MessageBusIpcCommon.h"
#include "PThreadLockGuard.h"
#include "SynchronizedChannelList.h"


/**
 * Iterator Constructor.
 * @param   channels MessageChannel list to iterate over
 * @param   mutex SynchronizedChannelList add/remove/iterate synchronization mutex
 * @brief   channellist_lock locks the mutex, so add/remove operations of SynchronizedChannelList are blocked
 */
SynchronizedChannelList::Iterator::Iterator(std::vector<MessageChannel> &channels, pthread_mutex_t &mutex) :
        channels(channels), channellist_lock(mutex) {

    // set iterator to starting position
    current_position = 0;
}

/**
 * Iterator Destructor.
 * @brief   channellist_lock gets destroyed and unlocks the mutex, so add/remove operations of SynchronizedChannelList get unlocked
 */
SynchronizedChannelList::Iterator::~Iterator() {
}

/**
 * @name    getNext
 * @brief   Get MessageChannel pointer and advance the iterator
 * @return  MessageChannel pointer if iterating not done yet, NULL otherwise
 */
MessageChannel const* SynchronizedChannelList::Iterator::getNext() {
    if (current_position == channels.size())
        return NULL;

    return &channels[current_position++];
}





/**
 * SynchronizedChannelList Constructor.
 * @brief   Initialize the add/remove/iterate mutex
 */
SynchronizedChannelList::SynchronizedChannelList() {
    pthread_mutex_init(&channels_mutex, NULL);
}

/**
 * SynchronizedChannelList Destructor.
 * @brief   Get rid of the add/remove/iterate mutex
 */
SynchronizedChannelList::~SynchronizedChannelList() {
    pthread_mutex_destroy(&channels_mutex);
}

/**
 * @name    add
 * @param   channel
 * @note    Thread safe add/remove/iterate
 */
void SynchronizedChannelList::add(MessageChannel &channel) {
    PThreadLockGuard lock(channels_mutex);

    channels.push_back(channel);
    DEBUG_MSG("%s: num channels: %d", __FUNCTION__, (int )channels.size());
}

/**
 * @name    remove
 * @param   index Index of item on the list to remove
 * @note    Thread safe add/remove/iterate
 */
void SynchronizedChannelList::remove(unsigned int index) {
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
void SynchronizedChannelList::removeByValue(MessageChannel &channel) {
    PThreadLockGuard lock(channels_mutex);

    channels.erase(std::remove(channels.begin(), channels.end(), channel), channels.end());
    DEBUG_MSG("%s: num channels: %d", __FUNCTION__, (int )channels.size());
}

/**
 * @name    getIterator
 * @return  MessageChannel iterator
 * @note    Thread safe add/remove/iterate
 */
SynchronizedChannelList::Iterator SynchronizedChannelList::getIterator() {
    return Iterator(channels, channels_mutex);
}
