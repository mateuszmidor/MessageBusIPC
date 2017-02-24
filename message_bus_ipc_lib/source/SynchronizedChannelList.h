/**
 *   @file: SynchronizedChannelList.h
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_SYNCHRONIZEDCHANNELLIST_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_SYNCHRONIZEDCHANNELLIST_H_

#include <vector>
#include "MessageChannel.h"
#include "PThreadLockGuard.h"


/**
 * @class   SynchronizedChannelList
 * @brief   List with thread-safe synchronized add/remove/iterate operations;
 *          add/remove blocks Iterator from being constructed
 *          Constructed Iterator object blocks add/remove operations.
 */
class SynchronizedChannelList {
public:
    class Iterator {
    public:
        Iterator(std::vector<MessageChannel> &channels, pthread_mutex_t &mtx);
        ~Iterator();
        MessageChannel const* getNext();

    private:
        std::vector<MessageChannel> &channels;
        unsigned int current_position;
        PThreadLockGuard channellist_lock; // this guy blocks add/remove operations of SynchronizedChannelList for the lifetime of Iterator
    };

public:
    SynchronizedChannelList();
    ~SynchronizedChannelList();

    void add(MessageChannel &channel);
    void remove(unsigned int index);
    void removeByValue(MessageChannel &channel);
    Iterator getIterator();

private:
    pthread_mutex_t channels_mutex;
    std::vector<MessageChannel> channels;
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_SYNCHRONIZEDCHANNELLIST_H_ */
