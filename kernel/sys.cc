#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"
#include "process.h"

int handleExit(uint32_t* frame) {
    // void exit(int status)
    return 0;
}

int handleWrite(uint32_t* frame) {
    // int  write(fd, buffer, len)
    return 0;
}

int handleFork() {
    // int fork()
    return 0;
}

int handleSem(uint32_t* frame) {
    // int sem(uint32_t init)
    return 0;
}

int handleUp(uint32_t* frame) {
    // int up(int s)
    return 0;
}

int handleDown(uint32_t* frame) {
    // int down(int s)
    return 0;
}

int handleClose(uint32_t* frame) {
    // int close(int id)
    return 0;
}

int handleShutdown() {
    // int shutdown(void)
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
    return 0;
}

int handleLen(uint32_t* frame) {
    // int len(int fs)
    return 0;
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
