#include <cstdio>
#include "forth.h"  // Forth front-end interface
#include "java.h"   // Java front-end interface

void send_to_console(int, const char* msg) { printf("%s", msg); }

int main(int ac, char* av[]) {
    if (ac <= 1) {
        fprintf(stderr,"Usage:> $0 file_name.class\n");
        return -1;
    }
    forth_setup(send_to_console);

    switch (java_setup(av[1], send_to_console)) {
    case -1: fprintf(stderr, " Failed to open file\n");       return -2;
    case -2: fprintf(stderr, " Failed to load class file\n"); return -3;
    }
    printf("\neJ32 v1 staring...\n");

    java_run();

    printf("\n\neJ32 done.\n");

    return 0;
}
