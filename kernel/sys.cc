#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    if (eax == 0) {
        // exit(int status)
        uint32_t status = *(frame+1);
        Debug::printf("| exiting with status %d\n", status);
        Debug::shutdown();
    } else if (eax == 1) {
        // write(fd, buffer, len)
        uint32_t* userStack = (uint32_t*) frame[3];
        //int32_t fd = userStack[1];
        char* buffer = (char*) userStack[2];
        uint32_t len = userStack[3];
        for (uint32_t i = 0; i < len; i++)
            Debug::printf("%c", *(buffer+i));
        return len;
    } else {
        Debug::panic("*** unrecognized system call %d\n", eax);
    }

    return 0;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
