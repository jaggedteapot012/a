#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "refs.h"
#include "atomic.h"

template <class T>
class StrongQueue {
    StrongPtr<T> head {};
    StrongPtr<T> tail {};
    SpinLock lock {};
public:
    StrongQueue() {}

    void add(StrongPtr<T> item) {
        lock.lock();
        item->next.reset();
        if (tail.isNull()) {
            head = item;
            tail = item;
        } else {
            tail->next = item;
            tail = item;
        }
        lock.unlock();
    }

    StrongPtr<T> remove() {
        lock.lock();
        StrongPtr<T> ptr = head;
        if (!ptr.isNull()) {
            head = ptr->next;
            if (head.isNull()) {
                tail.reset();
            }
            ptr->next.reset();
        }
        lock.unlock();
        return ptr;
    }
};

#endif
