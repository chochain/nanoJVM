#ifndef NANOJVM_UCODE_H
#define NANOJVM_UCODE_H
#include "core.h"

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
        (*(fop)(((uintptr_t)vt[c].xt)&~0x3))(t);
    }
};
extern Ucode gUcode;

#endif // NANOJVM_UCODE_H
