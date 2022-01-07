#ifndef NANOJVM_UCODE_H
#define NANOJVM_UCODE_H
#include "thread.h"       // include core.h, loader.h
#include "mmu.h"          // memory pool manager

#define OP_RETURN 0xb1
enum { DOVAR = 0, DOLIT, DOSTR, UNNEST };   /// Forth opcodes

struct Ucode {
    int     vtsz;
    Method  *vt;

    Ucode(U16 n, Method *uc) : vtsz(n), vt(uc) {}

    int find(const char *s) {
        for (int i = 0; i < vtsz; i++) {
            if (strcmp(s, vt[i].name)==0) return i;
        }
        return -1;
    }
    void exec(Thread &t, U32 c) {
#ifdef METHOD_PACKED
        (*(fop)(((uintptr_t)vt[c].xt)&~0x3))(t);
#else
        (*(vt[c].xt))(t);
#endif // METHOD_PACKED
    }
};
#endif // NANOJVM_UCODE_H
