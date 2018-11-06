#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "semaphore.h"

class Mutex : Semaphore {
public:
    Mutex() : Semaphore(1) {}
    void lock(void) { down(); }
    void unlock(void) { up(); }
};

class Locker {
    Mutex& theLock;
public:
    Locker(Mutex& m) : theLock(m) { theLock.lock(); }
    ~Locker() { theLock.unlock(); }

};

#endif
