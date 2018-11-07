#include "libc.h"

int main(int argc, char** argv) {
    printf("*** argc = %d\n",argc);
    for (int i=0; i<argc; i++) {
        printf("*** argv[%d]=%s\n",i,argv[i]);
    }

    uint32_t* ptr = (uint32_t*) 0xa0000000;
    printf("*** Memory should be zero filled: %lu\n", *ptr);

    cp(3, 1);

    return 0;
}
