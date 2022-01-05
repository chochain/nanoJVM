///
/// Forth microcode ROM and I/O functions
///
#ifndef NANOJVM_FORTH_H
#define NANOJVM_FORTH_H
#include "ucode.h"

char *next_word();
char *scan(char c);
void words(Thread &t);
void ss_dump(Thread &t);
void mem_dump(Thread &t, IU p0, DU sz);

void forth_setup(void (*callback)(int, const char*)=NULL);
void forth_interpreter(Thread &t);

#endif // NANOJVM_FORTH_H

