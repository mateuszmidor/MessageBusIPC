/**
 *   @file: PThreadLockGuard.h
 *
 *   @date: Feb 24, 2017
 * @author: Mateusz Midor
 */

#ifndef MESSAGE_BUS_IPC_LIB_SOURCE_PTHREADLOCKGUARD_H_
#define MESSAGE_BUS_IPC_LIB_SOURCE_PTHREADLOCKGUARD_H_

#include <pthread.h>

/**
 * @class   LockGuard
 * @brief   std::lock_guard for non-c++11 projects
 */
class PThreadLockGuard {
public:
    PThreadLockGuard(pthread_mutex_t &mutex);
    ~PThreadLockGuard();

private:
    pthread_mutex_t &mutex;
};

#endif /* MESSAGE_BUS_IPC_LIB_SOURCE_PTHREADLOCKGUARD_H_ */
