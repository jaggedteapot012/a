#ifndef _process_h
#define _process_h

#include "stdint.h"
#include "refs.h"
#include "semaphore.h"
#include "bobfs.h"
#include "mutex.h"

#define MAX_FDS 20

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
    Process* copy();
};

Process* activeProcess();

#endif
