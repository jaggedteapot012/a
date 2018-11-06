#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "bobfs.h"
#include "mutex.h"

void kernelStart(void);
void kernelMain(void);
void kernelTerminate(void);

struct KernelState {
    StrongPtr<BobFS> kernelFS;
    Mutex fsLock;

    KernelState() {}
    ~KernelState() {}
};

extern KernelState* kernelState;

#endif
