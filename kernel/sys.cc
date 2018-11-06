#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"
#include "process.h"
#include "bobfs.h"
#include "heap.h"
#include "libk.h"
#include "kernel.h"

#define assert Debug::assert
#define fs kernelState->kernelFS

char* parseString(const char* cur, char term) {
    const char* start = cur;
    int sz = 0; 

    int i = 0;
    while (cur[i] != term && cur[i] != '\0') {
        sz++; 
        i++;
    }

    auto str = new char[sz+1];
    cur = start;
    for (int i = 0; i < sz; i++)
        str[i] = cur[i];
    str[sz] = '\0';

    Debug::printf("*** Parsed string: '%s'\n", str);
    return str;
}

StrongPtr<Node> parseNode(const char* fn) {
    assert(fn[0] == '/', "Filepath must be absolute!");
    auto len = K::strlen(fn);

    // For parsing through file path.
    const char* curInd = fn+1;
    char* curName = parseString(curInd, '/');  // Read up to '/'
    auto curDir = BobFS::root(fs);

    auto curNode = curDir->findNode(curName);
    curInd += K::strlen(curName) + 1;

    while (curInd - fn < len) {
        // Create subdirectory if it does not exist.
        if (curNode.isNull()) {
            Debug::printf("*** directory %s does not exist, creating it!\n", curName);
            curNode = curDir->newDirectory(curName);
        }

        // Move to next file in path.
        free(curName);
        curName = parseString(curInd, '/');
        curInd += K::strlen(curName) + 1;

        // Find next node.
        curDir = curNode;
        curNode = curDir->findNode(curName);

    }

    if (curNode.isNull()) {
        Debug::printf("*** file %s does not exist, creating it!\n", curName);
        curNode = curDir->newFile(curName);
        assert(curNode->isFile(), "File not created!\n");
    }

    free(curName);
    return curNode;
}

int handleExit(uint32_t* frame) {
    // void exit(int status)
    return 0;
}

int handleWrite(uint32_t* frame) {
    // int write(fd, buffer, len)
    return 0;
}

int handleFork() {
    // int fork()
    return 0;
}

int handleSem(uint32_t* frame) {
    // int sem(uint32_t init)
    uint32_t init = frame[0];
    StrongPtr<Semaphore> sem { new Semaphore(init) };
    activeProcess()->newSem(sem);
    return 0;
}

int handleUp(uint32_t* frame) {
    // int up(int s)
    int fd = frame[0];
    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != sem_t)
        return -1;
    FD->semaphore->up();
    return 0;
}

int handleDown(uint32_t* frame) {
    // int down(int s)
    int fd = frame[0];
    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != sem_t)
        return -1;
    FD->semaphore->down();
    return 0;
}

int handleClose(uint32_t* frame) {
    // int close(int id)
    return 0;
}

int handleShutdown() {
    // int shutdown(void)
    Debug::shutdown();
    return 0;
}

int handleWait(uint32_t* frame) {
    // int wait(int id, uint32_t *ptr)
    return 0;
}

int handleExecl(uint32_t* frame) {
    // int execl(const char* path, const char* arg0, ...)
    return 0;
}

int handleOpen(uint32_t* frame) {
    // int open(const char* fn)
    kernelState->fsLock.lock();

    //auto path = parseString((char*) frame, '\0');
    auto node = parseNode("/randomDir/something");
    Debug::printf("*** isDir: %d\n", node->isDirectory());
    Debug::printf("*** isFile: %d\n", node->isFile());
    Debug::printf("*** links: %lu\n", node->getLinks());
    Debug::printf("*** size: %lu\n", node->getSize());

    //free(path);

    Debug::panic("");


    //StrongPtr<Node> f = parseNode(fileName);


    //StrongPtr<Node> file { new Semaphore(init) };
    //activeProcess()->newFile(sem);

    kernelState->fsLock.unlock();
    return 0;
}

int handleLen(uint32_t* frame) {
    // int len(int fs)
    int fd = frame[0];
    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != file_t)
        return -1;
    return FD->file->getSize();
}

int handleRead(uint32_t* frame) {
    // int read(int fd, void* buffer, size_t n)
    return 0;
}

int handleSeek(uint32_t* frame) {
    // int32_t seek(int fd, int32_t off)
    return 0;
}

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    frame = (uint32_t*) frame[3];
    if (eax == 0) {
        return handleExit(frame);
    } else if (eax == 1) {
        return handleWrite(frame);
    } else if (eax == 2) {
        return handleFork();
    } else if (eax == 3) {
        return handleSem(frame);
    } else if (eax == 4) {
        return handleUp(frame);
    } else if (eax == 5) {
        return handleDown(frame);
    } else if (eax == 6) {
        return handleClose(frame);
    } else if (eax == 7) {
        return handleShutdown();
    } else if (eax == 8) {
        return handleWait(frame);
    } else if (eax == 9) {
        return handleExecl(frame);
    } else if (eax == 10) {
        return handleOpen(frame);
    } else if (eax == 11) {
        return handleLen(frame);
    } else if (eax == 12) {
        return handleRead(frame);
    } else if (eax == 13) {
        return handleSeek(frame);
    } else {
        Debug::panic("*** unrecognized system call %d\n", eax);
    }
    return 0;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
