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
#define fsLock kernelState->fsLock

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

    //Debug::printf("*** Parsed string: '%s'\n", str);
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
            //Debug::printf("*** directory %s does not exist, creating it!\n", curName);
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
        //Debug::printf("*** file %s does not exist, creating it!\n", curName);
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
    Debug::panic("*** Calling write\n");
    return 0;
}

int handleFork() {
    // int fork()
    return 0;
}

int handleSem(uint32_t* frame) {
    // int sem(uint32_t init)
    uint32_t init = frame[1];
    StrongPtr<Semaphore> sem { new Semaphore(init) };
    int fd = activeProcess()->newSem(sem);
    return fd;
}

int handleUp(uint32_t* frame) {
    // int up(int s)
    int fd = frame[1];
    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != sem_t)
        return -1;
    FD->semaphore->up();
    return 0;
}

int handleDown(uint32_t* frame) {
    // int down(int s)
    int fd = frame[1];
    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != sem_t)
        return -1;
    FD->semaphore->down();
    return 0;
}

int handleClose(uint32_t* frame) {
    // int close(int id)
    Debug::panic("*** Calling close!\n");
    return 0;
}

int handleShutdown() {
    // int shutdown(void)
    Debug::shutdown();
    return 0;
}

int handleWait(uint32_t* frame) {
    // int wait(int id, uint32_t *ptr)
    Debug::panic("*** Calling wait!\n");
    return 0;
}

int handleExecl(uint32_t* frame) {
    // int execl(const char* path, const char* arg0, ...)
    return 0;
}

int handleOpen(uint32_t* frame) {
    // int open(const char* fn)

    auto path = parseString((char*) frame[1], '\0');
    auto node = parseNode(path);
    free(path);


    int32_t fd = activeProcess()->newFile(node);
    return fd;
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
    Debug::panic("*** Calling read!\n");
    return 0;
}

int handleSeek(uint32_t* frame) {
    // int32_t seek(int fd, int32_t off)
    Debug::panic("*** Calling seek!\n");
    return 0;
}

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    frame = (uint32_t*) frame[3];
    int ret;

    switch (eax) {
        case 0: ret = handleExit(frame); break;
        case 1: fsLock.lock(); ret = handleWrite(frame); fsLock.unlock(); break;
        case 2: ret = handleFork(); break;
        case 3: ret = handleSem(frame); break;
        case 4: ret = handleUp(frame); break;
        case 5: ret = handleDown(frame); break;

        // Unnecessary lock for semaphore, but probably fine.
        case 6: fsLock.lock(); ret = handleClose(frame); fsLock.unlock(); break;

        case 7: ret = handleShutdown(); break;
        case 8: ret =  handleWait(frame); break;
        case 9: ret = handleExecl(frame); break;
        case 10: fsLock.lock(); ret = handleOpen(frame); fsLock.unlock(); break;
        case 11: fsLock.lock(); ret = handleLen(frame); fsLock.unlock(); break;
        case 12: fsLock.lock(); ret = handleRead(frame); fsLock.unlock(); break;
        case 13: fsLock.lock(); ret = handleSeek(frame); fsLock.unlock();break;
        default:
            Debug::panic("*** unrecognized system call %d\n", eax);
    }
    return ret;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
