#ifndef NANOJVM_JAVA_H
#define NANOJVM_JAVA_H
///
/// Java front-end interface
///
int  java_setup(const char *fname, void (*callback)(int, const char*)=NULL);
void mem_stat();
void java_run();

#endif // NANOJVM_JAVA_H

