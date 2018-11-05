#ifndef _process_h
#define _process_h

#include "stdint.h"
#include "refs.h"
#include "semaphore.h"
#include "bobfs.h"

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
    union data {
        StrongPtr<Node> file;
        StrongPtr<Semaphore> semaphore;
    };
};

struct Process {
    FileDescriptor fds[20];
};

Process* activeProcess();

#endif
