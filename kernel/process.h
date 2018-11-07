#ifndef _process_h
#define _process_h

#include "stdint.h"
#include "refs.h"
#include "semaphore.h"
#include "bobfs.h"
#include "mutex.h"

// 10 sem, 10 files, but 0-2 are reserved
#define MAX_FDS 30

enum Filetype {
    EMPTY,
    STDIN,
    STDOUT,
    STDERR,
    file_t,
    sem_t
};

struct FileDescriptor {
    Filetype filetype;
    StrongPtr<Node> file;
    StrongPtr<Semaphore> semaphore;
    uint32_t offset;
};

class Process {
    int32_t allocFD(bool dir);
    FileDescriptor fds[MAX_FDS];
    Mutex pLock;
public:
    Process();
    FileDescriptor* getFD(int32_t fd);
    int32_t closeFD(int32_t fd);
    int32_t newFile(StrongPtr<Node> file);
    int32_t newSem(StrongPtr<Semaphore> sem);
    int32_t pid;
    static Atomic<int32_t> pidAllocator;
    StrongPtr<Process> copy();
};

StrongPtr<Process> activeProcess();

#endif
