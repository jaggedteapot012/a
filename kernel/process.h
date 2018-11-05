#ifndef _process_h
#define _process_h

#include "stdint.h"
#include "refs.h"
#include "semaphore.h"
#include "bobfs.h"

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
};

struct Process {
    FileDescriptor fds[MAX_FDS];
};

Process* activeProcess();
FileDescriptor* getFD(int32_t fd);

#endif
