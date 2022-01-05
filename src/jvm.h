#ifndef NANOJVM_JVM_H
#define NANOJVM_JVM_H
///
/// VM I/O functions
///
int  jvm_setup(const char *fname, void (*callback)(int, const char*)=NULL);
void mem_stat();
void jvm_run();

#endif // NANOJVM_JVM_H

