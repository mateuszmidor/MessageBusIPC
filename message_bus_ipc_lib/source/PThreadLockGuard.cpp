/**
 *   @file: PThradLockGuard.cpp
 *
 *   @date: Feb 24, 2017
 * @author: Mateusz Midor
 */

#include "PThreadLockGuard.h"

PThreadLockGuard::PThreadLockGuard(pthread_mutex_t &mutex) : mutex(mutex) {
    pthread_mutex_lock(&mutex);
}

PThreadLockGuard::~PThreadLockGuard() {
    pthread_mutex_unlock(&mutex);
}
