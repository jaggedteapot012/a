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
#include "threads.h"
#include "future.h"
#include "elf.h"

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
        // O_CREAT is false, fail if file does not exist.
        if (curNode.isNull()) {
            return curNode;
            //Debug::printf("*** directory %s does not exist, creating it!\n", curName);
            //curNode = curDir->newDirectory(curName);
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
        // O_CREAT is false, fail if file does not exist.
        return curNode;
        //Debug::printf("*** file %s does not exist, creating it!\n", curName);
        //curNode = curDir->newFile(curName);
    }

    free(curName);
    return curNode;
}

int handleExit(uint32_t* frame) {
    // void exit(int status)
    int32_t status = frame[1];
    activeProcess()->setStatus(status);
    stop();
    return 0;
}

int handleWrite(uint32_t* frame) {
    // int write(fd, buffer, nbytes)
    int fd = frame[1];
    void* buf = (void*) frame[2];
    uint32_t len = frame[3];

    uint32_t bytesWritten = 0;
    
    if (fd == 1 || fd == 2) {
        // For writing to stdout and stderr.
        char* buffer = (char*) buf;
        while (bytesWritten < len) {
            Debug::printf("%c", buffer[bytesWritten++]);
        }
    } else {
        // For writing to a file.
        FileDescriptor* FD = activeProcess()->getFD(fd);
        if (FD == nullptr || FD->filetype != file_t)
            return -1;
        bytesWritten = FD->file->writeAll(FD->offset, buf, len);
    }

    return (int) bytesWritten;
}

// there's probably a cleaner way to do this, but it works sooooo
template <typename T>
void createChild(T work, Future<int32_t>& childPid) { 
    reaper();
    auto child = new ThreadImpl<T>(work, active());
    long *topOfStack = &child->stack[2045];
    topOfStack[0] = 0x200;       // sti
    topOfStack[1] = 0;           // cr2
    topOfStack[2] = (long)entry;
    child->esp = (long) topOfStack;
    schedule(child);
    
    childPid.set(child->process->pid);
}

int handleFork(uint32_t* intFrame) {
    // int fork()
    uint32_t pc = intFrame[0];
    uint32_t esp = intFrame[3];
    Future<int32_t> childPid;
    createChild([pc, esp]() {
        switchToUser(pc, esp, 0);
    }, childPid);

    return childPid.get();
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
    int fd = frame[1];

    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || (FD->filetype != file_t && FD->filetype != sem_t))
        return -1;

    // Free index in FD table.
    return activeProcess()->closeFD(fd);
}

int handleShutdown() {
    // int shutdown(void)
    // TODO: fix this depending on piazza answer
    Debug::printf("*** 2000000000 shutdown\n");
    Debug::shutdown();
    return 0;
}

int handleWait(uint32_t* frame) {
    // int wait(int id, int32_t *ptr)
    uint32_t pid = frame[1];
    int32_t* ptr = (int32_t*) frame[2];
    return Process::getStatus(pid, ptr);
}

int handleExecl(uint32_t* frame) {
    // int execl(const char* path, const char* arg0, ...)
    const char* path = (char*) frame[1];
    StrongPtr<Node> exec = parseNode(path);
    if (exec.isNull())
        return -1;

    /* drop existing user-space mappings
     * load user-program
     * push arguments on stack */
    uint32_t i = 2;
    while (frame[i] != 0) i++;
    uint32_t numArgs = i-2;
    char* args[numArgs];
    for (i = 2; i < 2+numArgs; i++) {
        args[i-2] = parseString((char*) frame[i], 0);
    }
    
    Thread* me = active();
    me->addressSpace->erase();
    uint32_t e = ELF::load(exec);
    char* topOfStack = (char*) 0xeffff000;
    char* userArgs[numArgs];
    for (i = 0; i < numArgs; i++) {
        char* str = args[i];
        uint32_t len = K::strlen(str)+1;
        topOfStack -= len;
        userArgs[i] = topOfStack+1;
        memcpy(topOfStack+1, str, len);
        free(str);
    }

    uint32_t* stack = (uint32_t*) ((uint32_t)topOfStack - ((uint32_t)topOfStack%4)-4);
    stack -= numArgs-1;
    for (i = 0; i < numArgs; i++) {
        stack[i] = (uint32_t) userArgs[i];
    }
    stack -= 1;
    *stack = (uint32_t) (stack+1);
    stack -= 1;
    *stack = numArgs;
    switchToUser(e, (uint32_t)stack, 0);
    
    assert(false, "failed to switch to user");
    return 0;
}

int handleOpen(uint32_t* frame) {
    // int open(const char* fn)

    auto path = (char*) frame[1];
    auto node = parseNode(path);

    // File did not exist.
    if (node.isNull())
        return -1;

    int32_t fd = activeProcess()->newFile(node);
    return fd;
}

int handleLen(uint32_t* frame) {
    // int len(int fs)
    int fd = frame[1];
    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != file_t)
        return -1;
    return FD->file->getSize();
}

int handleRead(uint32_t* frame) {
    // int read(int fd, void* buffer, size_t n)
    int fd = frame[1];
    void* buf = (void*) frame[2];
    uint32_t len = frame[3];

    // Cannot read from STDIN/STDOUT
    if (fd < 3)
        return -1;

    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != file_t)
        return -1;

    int bytesRead = FD->file->readAll(FD->offset, buf, len);

    FD->offset += bytesRead;

    return bytesRead;
}

int handleSeek(uint32_t* frame) {
    // int32_t seek(int fd, int32_t off)
    int fd = frame[1];
    int off = frame[2];

    // Cannot seek in STDIN/STDOUT
    if (fd < 3)
        return -1;

    FileDescriptor* FD = activeProcess()->getFD(fd);
    if (FD == nullptr || FD->filetype != file_t)
        return -1;

    FD->offset = off;

    return off;
}

extern "C" int sysHandler(uint32_t eax, uint32_t *intFrame) {
    uint32_t* frame = (uint32_t*) intFrame[3];
    int ret;

    switch (eax) {
        case 0: ret = handleExit(frame); break;
        case 1: fsLock.lock(); ret = handleWrite(frame); fsLock.unlock(); break;
        case 2: ret = handleFork(intFrame); break;
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
