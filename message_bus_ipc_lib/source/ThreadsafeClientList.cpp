/*
 * ThreadsafeClientList.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: mateusz
 */

#include "PThreadLockGuard.h"
#include "ThreadsafeClientList.h"

namespace messagebusipc {

ThreadsafeClientList::ThreadsafeClientList() {
    pthread_mutex_init(&mutex, NULL);
}

ThreadsafeClientList::~ThreadsafeClientList() {
    pthread_mutex_destroy(&mutex);
}

/**
 * @name    update
 * @note    Thread safe
 */
void ThreadsafeClientList::update(const std::string &client_names) {
    PThreadLockGuard lock(mutex);

    clients = client_names;
}

/**
 * @name    exists
 * @note    Thread safe
 */
bool ThreadsafeClientList::exists(const std::string &client_name) {
    PThreadLockGuard lock(mutex);

    return (clients.find(client_name, 0) != std::string::npos);
}

} /* namespace messagebusipc */
