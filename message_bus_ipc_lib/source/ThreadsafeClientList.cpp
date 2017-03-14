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
 * @name    add
 * @note    Thread safe
 */
void ThreadsafeClientList::add(const std::string &client_name) {
    PThreadLockGuard lock(mutex);

    clients += client_name + ";";
}
/**
 * @name    remove
 * @note    Thread safe
 */
void ThreadsafeClientList::remove(const std::string &client_name) {
    PThreadLockGuard lock(mutex);

    std::string::size_type i = clients.find(client_name);

    if (i != std::string::npos)
       clients.erase(i, client_name.length() + 1 ); // +1 for ';'
}

/**
 * @name    clear
 * @note    Thread safe
 */
void ThreadsafeClientList::clear() {
    PThreadLockGuard lock(mutex);

    clients.clear();
}

/**
 * @name    exists
 * @note    Thread safe
 */
bool ThreadsafeClientList::exists(const std::string &client_name) {
    PThreadLockGuard lock(mutex);

    return (clients.find(client_name) != std::string::npos);
}

/**
 * @name    toString
 * @brief   Return available clients in a single string
 * @note    Thread safe
 */
std::string ThreadsafeClientList::toString() {
    PThreadLockGuard lock(mutex);

    return clients;
}
} /* namespace messagebusipc */
