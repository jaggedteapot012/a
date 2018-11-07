#include "process.h"
#include "threads.h"
#include "debug.h"
#include "mutex.h"

// Initialize bump allocator for pids.
#define PID_START 1000  // start at 1000 so it's obviously a PID
Atomic<int32_t> Process::pidAllocator {0};

StrongPtr<Process> activeProcess() {
    auto thread = active();
    if (thread->process.isNull()) {
        thread->process = StrongPtr<Process> { new Process() };
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

    // Initialize PID.
    pid = Process::pidAllocator.fetch_add(1) + PID_START;
}

FileDescriptor* Process::getFD(int32_t fd) {
    if (fd < 0 || fd >= MAX_FDS) {
        return nullptr;
    }
    return fds+fd;
}

int32_t Process::allocFD(bool isSem) {
    // Sems start at higher fd value.

    uint32_t min = isSem ? MAX_FDS - 10 : 0;
    uint32_t max = isSem ? MAX_FDS : MAX_FDS - 10;
    for (uint32_t i = min; i < max; i++)
        if (fds[i].filetype == EMPTY)
            return i;

    Debug::panic("*** ran out of file descriptors\n");
    return -1;
}

int32_t Process::newFile(StrongPtr<Node> file) {
    Locker x(pLock);

    int32_t fd = allocFD(false);    
    fds[fd].filetype = file_t;
    fds[fd].file = file;
    return fd;
}

int32_t Process::newSem(StrongPtr<Semaphore> sem) {
    Locker x(pLock);
    int32_t fd = allocFD(true);    
    fds[fd].filetype = sem_t;
    fds[fd].semaphore = sem;
    return fd;
}

int32_t Process::closeFD(int32_t fd) {
    Locker x(pLock);

    // Find fd, return -1 if not exist or already empty.
    auto FD = getFD(fd);
    if (FD == nullptr || FD->filetype == EMPTY) {
        return -1;
    }

    FD->file.reset();
    FD->semaphore.reset();
    FD->filetype = EMPTY;
    FD->offset = 0;

    return 0;
}

StrongPtr<Process> Process::copy() {
    StrongPtr<Process> result = StrongPtr<Process> { new Process() };
    for (uint32_t i = 0; i < MAX_FDS; i++) {
        result->fds[i].filetype = fds[i].filetype;
        result->fds[i].file = fds[i].file;
        result->fds[i].semaphore = fds[i].semaphore;
        result->fds[i].offset = fds[i].offset;
    }
    return result;
}
