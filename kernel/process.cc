#include "process.h"
#include "threads.h"
#include "debug.h"

Process* activeProcess() {
    return active()->process;
}

FileDescriptor* getFD(int32_t fd) {
    if (fd < 0 || fd >= MAX_FDS)
        return nullptr;
    return (activeProcess()->fds)+fd;
}
