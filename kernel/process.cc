#include "process.h"
#include "threads.h"
#include "debug.h"

Process* activeProcess() {
    return active()->process;
}

FileDescriptor* Process::getFD(int32_t fd) {
    if (fd < 0 || fd >= MAX_FDS)
        return nullptr;
    return fds+fd;
}

int32_t Process::allocFD() {
    for (uint32_t i = 0; i < MAX_FDS; i++)
        if (fds[i].filetype == EMPTY)
            return i;

    Debug::panic("*** ran out of file descriptors\n");
    return -1;
}

int32_t Process::newFile(StrongPtr<Node> file) {
    int32_t fd = allocFD();    
    fds[fd].filetype = file_t;
    fds[fd].file = file;
    return fd;
}

int32_t Process::newSem(StrongPtr<Semaphore> sem) {
    int32_t fd = allocFD();    
    fds[fd].filetype = sem_t;
    fds[fd].semaphore = sem;
    return fd;
}
