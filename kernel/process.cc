#include "process.h"
#include "threads.h"

Process* activeProcess() {
    return active()->process;
}
