///
/// Forth I/O functions
///
#ifndef NANOJVM_FORTH_IO_H
#define NANOJVM_FORTH_IO_H
#include "ucode.h"
#include "thread.h"
#include "mmu.h"

char *next_word();
char *scan(char c);
void words(Thread &t);
void ss_dump(Thread &t);
void mem_dump(Thread &t, IU p0, DU sz);
void forth_interpreter(Thread &t);

#endif // NANOJVM_FORTH_IO_H

