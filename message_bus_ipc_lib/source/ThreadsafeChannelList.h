/**
 *   @file: ThreadsafeChannelList.h
 *
 *   @date: Feb 23, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFECHANNELLIST_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFECHANNELLIST_H_

#include <vector>
#include "MessageChannel.h"
#include "PThreadLockGuard.h"

namespace messagebusipc {

/**
 * @class   ThreadsafeChannelList
 * @brief   List with thread-safe add/remove/iterate operations;
 *          add/remove blocks Iterator from being constructed
 *          Constructed Iterator object blocks add/remove operations.
 */
class ThreadsafeChannelList {
public:
    class Iterator {
    public:
        Iterator(std::vector<MessageChannel> &channels, pthread_mutex_t &mtx);
        ~Iterator();
        MessageChannel const* getNext();
        void reset();

    private:
        std::vector<MessageChannel> &channels;
        unsigned int current_position;
        PThreadLockGuard channellist_lock; // this guy blocks add/remove operations of ThreadsafeChannelList for the lifetime of Iterator
    };

public:
    ThreadsafeChannelList();
    ~ThreadsafeChannelList();

    void add(MessageChannel &channel);
    void remove(unsigned int index);
    void removeByValue(MessageChannel &channel);
    Iterator getIterator();

private:
    pthread_mutex_t channels_mutex;
    std::vector<MessageChannel> channels;
};

}

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFECHANNELLIST_H_ */
