#ifndef NANOJVM_JAVA_H
#define NANOJVM_JAVA_H
///
/// Java front-end interface
///
int  java_setup(void (*callback)(int, const char*)=NULL);
int  java_load(const char *fname);
void java_run();         // virtual function

#endif // NANOJVM_JAVA_H

