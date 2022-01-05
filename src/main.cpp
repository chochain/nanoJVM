#include <cstdio>
#include "forth.h"
#include "jvm.h"

void send_to_console(int, const char* msg) { printf("%s", msg); }

int main(int ac, char* av[]) {
    if (ac <= 1) {
        fprintf(stderr,"Usage:> $0 file_name.class\n");
        return -1;
    }
    forth_setup(send_to_console);

    switch (jvm_setup(av[1], send_to_console)) {
    case -1: fprintf(stderr, " Failed to open file\n");       return -2;
    case -2: fprintf(stderr, " Failed to load class file\n"); return -3;
    }
    printf("\neJ32 v1 staring...\n");

    jvm_run();

    printf("\n\neJ32 done.\n");

    return 0;
}
