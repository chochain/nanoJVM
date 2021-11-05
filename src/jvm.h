#ifndef NANOJVM_JVM_H
#define NANOJVM_JVM_H
#include "core.h"
///
/// VM I/O functions
///
char *next_word();
char *scan(char c);
void words(Thread &t);
void ss_dump(Thread &t);
void mem_dump(Thread &t, IU p0, DU sz);
void forth_interpreter(Thread &t);

int  jvm_setup(const char *fname);
void jvm_run();

#endif // NANOJVM_JVM_H

