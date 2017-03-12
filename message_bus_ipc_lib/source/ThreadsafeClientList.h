/*
 * ThreadsafeClientList.h
 *
 *  Created on: Feb 28, 2017
 *      Author: mateusz
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFECLIENTLIST_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFECLIENTLIST_H_

#include <string>
#include <pthread.h>

namespace messagebusipc {

/**
 * @class   ThreadsafeClientList
 * @brief   Thread safe list of connected client names
 */
class ThreadsafeClientList {
public:
    ThreadsafeClientList();
    virtual ~ThreadsafeClientList();

    void add(const std::string &client_name);
    void remove(const std::string &client_name);
    void clear();
    bool exists(const std::string &client_name);
private:
    pthread_mutex_t mutex;
    std::string clients;
};

} /* namespace messagebusipc */

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_THREADSAFECLIENTLIST_H_ */
