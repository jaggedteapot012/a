#include "process.h"
#include "threads.h"
#include "debug.h"
#include "mutex.h"
#include "heap.h"

// Initialize bump allocator for pids.
#define PID_START 1000  // start at 1000 so it's obviously a PID
Atomic<int32_t> Process::pidAllocator {0};
Future<int32_t>* Process::statuses[MAX_PROCESSES];

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
    statuses[pid-PID_START] = new Future<int32_t>();
    if (pid >= (PID_START+MAX_PROCESSES)) {
        Debug::panic("*** no more available processes\n");
    }
}

int32_t Process::getStatus(uint32_t pid, int32_t* ptr) {
    uint32_t index = pid - PID_START;
    if (index >= MAX_PROCESSES || statuses[index] == nullptr)
        return -1;
    *ptr = statuses[index]->get();
    return 0;
}

void Process::setStatus(int32_t status) {
    uint32_t index = pid - PID_START;
    statuses[index]->set(status);
}

FileDescriptor* Process::getFD(int32_t fd) {
    if (fd < 0 || fd >= MAX_FDS) {
        return nullptr;
    }
    return fds+fd;
}

int32_t Process::allocFD(bool dir) {
    // If dir (direction) is true, find lowest available, 
    // otherwise highest. Needed because t0 requires first 
    // file to be fd 3, but sem is called first.

    if (dir) {
        for (uint32_t i = 0; i < MAX_FDS; i++)
            if (fds[i].filetype == EMPTY)
                return i;
    } else {
        for (uint32_t i = MAX_FDS-1; i >= 0; i--)
            if (fds[i].filetype == EMPTY)
                return i;
    }

    Debug::panic("*** ran out of file descriptors\n");
    return -1;
}

int32_t Process::newFile(StrongPtr<Node> file) {
    Locker x(pLock);

    int32_t fd = allocFD(true);    
    fds[fd].filetype = file_t;
    fds[fd].file = file;
    return fd;
}

int32_t Process::newSem(StrongPtr<Semaphore> sem) {
    Locker x(pLock);
    int32_t fd = allocFD(false);    
    fds[fd].filetype = sem_t;
    fds[fd].semaphore = sem;
    return fd;
}

int32_t Process::closeFD(int32_t fd) {
    Locker x(pLock);

    // Find fd, return -1 if not exist or invalid.
    auto FD = getFD(fd);
    if (FD == nullptr || (FD->filetype != file_t && FD->filetype != sem_t)) {
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
