#ifndef _process_h
#define _process_h

#include "stdint.h"
#include "refs.h"
#include "semaphore.h"
#include "bobfs.h"
#include "mutex.h"
#include "future.h"

// 10 sem, 10 files, but 0-2 are reserved
#define MAX_FDS 30
#define MAX_PROCESSES 100

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
    static Future<int32_t>* statuses[MAX_PROCESSES];
    int32_t allocFD(bool dir);
    FileDescriptor fds[MAX_FDS];
    Mutex pLock;
public:
    static Atomic<int32_t> pidAllocator;
    static int32_t getStatus(uint32_t pid, int32_t* ptr);
    void setStatus(int32_t status);

    int32_t pid;
    FileDescriptor* getFD(int32_t fd);
    int32_t closeFD(int32_t fd);
    int32_t newFile(StrongPtr<Node> file);
    int32_t newSem(StrongPtr<Semaphore> sem);
    Process();
    StrongPtr<Process> copy();
};

StrongPtr<Process> activeProcess();

#endif
