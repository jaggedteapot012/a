#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    if (eax == 0) {
        // void exit(int status)
        uint32_t status = *(frame+1);
        Debug::printf("| exiting with status %d\n", status);
        Debug::shutdown();
    } else if (eax == 1) {
        // int  write(fd, buffer, len)
        uint32_t* userStack = (uint32_t*) frame[3];
        //int32_t fd = userStack[1];
        char* buffer = (char*) userStack[2];
        uint32_t len = userStack[3];
        for (uint32_t i = 0; i < len; i++)
            Debug::printf("%c", *(buffer+i));
        return len;
    } else if (eax == 2) {
        // int fork()
    } else if (eax == 3) {
        // int sem(uint32_t init)
    } else if (eax == 4) {
        // int up(int s)
    } else if (eax == 5) {
        // int down(int s)
    } else if (eax == 6) {
        // int close(int id)
    } else if (eax == 7) {
        // int shutdown(void)
    } else if (eax == 8) {
        // int wait(int id, uint32_t *ptr)
    } else if (eax == 9) {
        // int execl(const char* path, const char* arg0, ...)
    } else if (eax == 10) {
        // int open(const char* fn)
    } else if (eax == 11) {
        // int len(int fs)
    } else if (eax == 12) {
        // int read(int fd, void* buffer, size_t n)
    } else if (eax == 13) {
        // int32_t seek(int fd, int32_t off)
    } else {
        Debug::panic("*** unrecognized system call %d\n", eax);
    }

    return 0;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
