#include <cstdio>
#include "forth.h"  // Forth front-end interface
#include "java.h"   // Java front-end interface

#ifndef ARDUINO
void send_to_console(int, const char* msg) { printf("%s", msg); }

int main(int ac, char* av[]) {
    if (ac <= 1) {
        fprintf(stderr,"Usage:> $0 file_name.class\n");
        return -1;
    }
    forth_setup(send_to_console);
    java_setup(send_to_console);

    for (int i=1; i<ac; i++) {
    	if (!java_load(av[i])) {
    		fprintf(stderr, " Failed to load class file: %s\n", av[i]);
    		return -2;
    	}
    }
    printf("\neJ32 v1 staring...\n");

    java_run();

    printf("\n\neJ32 done.\n");

    return 0;
}
#endif // ARDUINO
