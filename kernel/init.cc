#include "init.h"

#include "debug.h"
#include "config.h"
#include "u8250.h"
#include "smp.h"
#include "machine.h"
#include "heap.h"
#include "idt.h"

bool lapicFound = false;

constexpr uint32_t STACK_BYTES = 8 * 1024;

constexpr uint32_t stack0 = 0x100000;
constexpr uint32_t heapStart = stack0 + STACK_BYTES;

extern "C" uint32_t pickKernelStack(void) {
    uint32_t me = lapicFound ? SMP::me() : 0;

    if (me == 0) { return stack0 + STACK_BYTES; }
    else { return ((uint32_t)malloc(STACK_BYTES)) + STACK_BYTES; }
}

extern "C" void kernelInit(void) {
        U8250 temp_uart; // until we have a heap
        Debug::init(&temp_uart);
        Debug::debugAll = false;
        Debug::printf("\n| What just happened? Why am I here?\n");

        /* discover configuration */
        configInit(&kConfig);
        Debug::printf("| totalProcs %d\n",kConfig.totalProcs);
        Debug::printf("| memSize 0x%x %dMB\n",
            kConfig.memSize,
            kConfig.memSize / (1024 * 1024));
        Debug::printf("| localAPIC %x\n",kConfig.localAPIC);
        Debug::printf("| ioAPIC %x\n",kConfig.ioAPIC);

        /* initialize the heap */
        uint32_t heapSize = (8 << 20) - STACK_BYTES;     // 8M 
        heapInit((void*)heapStart,heapSize);

        /* now we can allocate the real UART */
        Debug::init(new U8250);

        MISSING();
}
