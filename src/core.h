#ifndef NANOJVM_CORE_H
#define NANOJVM_CORE_H
#include "common.h"

struct Thread;
typedef void (*fop)(Thread&); /// opcode function pointer
///
/// Method class
///
#define FLAG_IMMD   0x1
#define FLAG_FORTH  0x2
#define FLAG_JAVA   0x4
struct Method {
    const char *name = 0;     /// for debugging, TODO (in const_pool)
#if METHOD_PACKED
    union {
        fop   xt = 0;         /// function pointer (or decayed lambda)
        struct {
            U32 flag: 3;
            U32 rsv:  29;
        };
    };
#else
    fop   xt   = 0;           /// function pointer (or decayed lambda)
    U8    flag = 0;
    U8    parm = 0;
#endif
    Method(const char *n, fop f, U32 im=0, U32 pm=0) : name(n), xt(f), flag(im), parm(pm) {}
};
///
/// Word - shared struct for Class and Method
///   class list - linked list of words, dict[cls_root], pfa => next_class
///   vtable     - linked list of words, dict[class.pfa], pfa => next_method
///
#define CLS_SUPR        0
#define CLS_INTF        2
#define CLS_VT          4
#define CLS_CVSZ        6
#define CLS_IVSZ        8
#define CLS_CV          10
#define MTH_PARM        sizeof(PU)
struct Word {                /// 4-byte header
    IU  lfa;                 /// link field to previous word
    U8  len;                 /// name of method

    U8  immd:   1;           /// Forth immediate word
    U8  forth:  1;           /// 0:native, 1:composite
    U8  java:   1;           /// Java method
    U8  access: 2;           /// public, private, protected
    U8  ftype:  3;           /// static, final, virtual, synchronized

    U8  data[];              /// name field + parameter field

    char *nfa()         { return (char*)&data[0];  }
    U8   *pfa(U8 off=0) { return &data[len + off]; }
};
#endif // NANOJVM_CORE_H
