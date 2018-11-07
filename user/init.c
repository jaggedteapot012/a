#include "libc.h"

void check(int res, const char* msg) {
    if (res < 0)
        printf("*** %s\n", msg);
}
void one(int fd) {
    printf("*** fd = %d\n",fd);
    printf("*** len = %d\n",len(fd));

    /* Check stdout instead of stderr. */
    cp(fd, 1);
}

int main(int argc, char** argv) {
    // Check open, len, read, write to stdout.
    int fd = open("/etc/extra.txt",0);
    check(fd, "failed to open file!");
    one(fd);

    // Check seeking.
    printf("*** Seeking to %d\n", (int) seek(fd, 10));
    cp(fd, 1);

    // Check close.
    check(close(fd), "failed to close!");

    // Fork/semaphore test.
    int s = sem(0);
    int pid = fork();
    check(pid, "failed to fork!");

    if (pid == 0) {
        /* Child */

        // Ensure both child, grandchild and parent have the same fd table and semaphore.
        // Ensure nested forks work.
        int pid2 = fork();
        check(pid, "failed to fork again!");
        if (pid2 == 0) {
            /* Child */
            up(s);
            exit(1);
        } else {
            /* Parent */
            up(s);
            exit(15);
        }

    } else {
        /* Parent */
        down(s);
        down(s);
        printf("*** Got message from children\n");

        /* do some indeterminate waiting */
        for (int i = 0; i < 12345; i++);
        for (int i = 12345; i > 0; i--);

        /* make sure we can still wait on finished child. */
        uint32_t rc;
        wait(pid, &rc);
        printf("*** 1+14=%lu\n", rc);
    }

    /* Test that execl actually clears virtual address space. */
    uint32_t* ptr = (uint32_t*) 0xa0000000;
    *ptr = 123;

    /* Test that execl keeps fd table. */
    fd = open("/etc/extra.txt",0);
    check(fd, "failed to open file!");
    check(seek(fd, 20), "failed to seek!");

    char extra[14] = "*** OrEaDlY?\n";

    check(write(fd, &extra, 13), "failed to write!");
    check(seek(fd, 0), "failed to seek again!\n");

    cp(fd, 1);
    shutdown();


    /* Exec into shell.c */
    check(execl("/sbin/shell", "some", "args", 0), "execl failed!");
    return 0;
}
