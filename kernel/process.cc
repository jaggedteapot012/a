#include "process.h"
#include "threads.h"
#include "debug.h"

Process* activeProcess() {
    auto thread = active();
    if (thread->process == nullptr) {
        thread->process = new Process();
    }
    return thread->process;
}

Process::Process() : pLock() {
    // Initialize std fds.
    fds[0].filetype = STDIN;
    fds[1].filetype = STDOUT;
    fds[2].filetype = STDERR;

    // Clear rest of array.
    for (int i = 3; i < MAX_FDS; i++) 
        fds[i].filetype = EMPTY;

}

FileDescriptor* Process::getFD(int32_t fd) {
    if (fd < 0 || fd >= MAX_FDS) {
        return nullptr;
    }
    return fds+fd;
}

int32_t Process::allocFD() {
    for (uint32_t i = 0; i < MAX_FDS; i++)
        if (fds[i].filetype == EMPTY) {
            return i;
        }

    Debug::panic("*** ran out of file descriptors\n");
    return -1;
}

int32_t Process::newFile(StrongPtr<Node> file) {
    pLock.lock();
    int32_t fd = allocFD();    
    fds[fd].filetype = file_t;
    fds[fd].file = file;
    pLock.unlock();
    return fd;
}

int32_t Process::newSem(StrongPtr<Semaphore> sem) {
    pLock.lock();
    int32_t fd = allocFD();    
    fds[fd].filetype = sem_t;
    fds[fd].semaphore = sem;
    pLock.unlock();
    return fd;
}

Process* Process::copy() {
    Process* result = new Process();
    memcpy(result->fds, fds, MAX_FDS*sizeof(FileDescriptor));
    return result;
}
