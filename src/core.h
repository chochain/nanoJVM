#ifndef NANOJVM_CORE_H
#define NANOJVM_CORE_H
#include "common.h"
///
/// array class template (so we don't have dependency on C++ STL)
/// Note:
///   * using decorator pattern
///   * this is similar to vector class but much simplified
///   * v array is dynamically allocated due to ESP32 has a 96K hard limit
///
template<class T, int N>
struct List {
    T   *v;             /// fixed-size array storage
    int idx = 0;        /// current index of array
    int max = 0;        /// high watermark for debugging

    List()  { v = new T[N]; }      /// dynamically allocate array memory
    ~List() { delete[] v;   }      /// free the memory
    T& operator[](int i)   { return i < 0 ? v[idx + i] : v[i]; }
#if RANGE_CHECK
    T pop() {
        if (idx>0) return v[--idx];
        throw "ERR: List empty";
    }
    int push(T t) {
        if (idx<N) { v[max=idx] = t; return idx++; }
        throw "ERR: List full";
    }
#else
    T   pop()     { return v[--idx]; }
    int push(T t) { v[max=idx] = t; return idx++; }
#endif // RANGE_CHECK
    void push(T *a, int n)  { for (int i=0; i<n; i++) push(*(a+i)); }
//    void merge(List& a)     { for (int i=0; i<a.idx; i++) push(a[i]);}
//    void clear(int i=0)     { idx=i; }
};
///
/// thread class forward declaration
///
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
