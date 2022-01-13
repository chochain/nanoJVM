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
#define ACL_PUBLIC  0x00
#define ACL_PRIVATE 0x01
#define ACL_PROTECT 0x02
#define ACL_BUILTIN 0x03
#define FORTH_FUNC  0x20
#define JAVA_FUNC   0x40
#define IMMD_FLAG   0x80
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
    fop         xt   = 0;     /// function pointer (or decayed lambda)
    U8          flag = 0;
    const char *parm = 0;
#endif
    Method(const char *n, fop f, U32 im=0, const char *pm=0) : name(n), xt(f), flag(im), parm(pm) {}
};
#define VTSZ(vt)       (sizeof(vt)/sizeof(Method))
///
/// Word - shared struct for Class and Method
///   class list - linked list of words, dict[cls_root], pfa => next_class
///   vtable     - linked list of words, dict[class.pfa], pfa => next_method
///
#define PFA_CLS_SUPR    0    /** super class            */
#define PFA_CLS_JDX     2    /** java class file index  */
#define PFA_CLS_VT      4    /** java virtual table     */
#define PFA_CLS_CVSZ    6    /** class variable count   */
#define PFA_CLS_IVSZ    8    /** instance var count     */
#define PFA_CLS_CV      10   /** class variable storage */
#define PFA_PARM_IDX    sizeof(PU)
struct Word {                /// 4-byte header
    IU  lfa;                 /// link field to previous word
    U8  len;                 /// name of method

    U8  access: 2;           /// public, private, protected, built-in
    U8  ftype:  3;           /// static, final, virtual, synchronized
    U8  forth:  1;           /// 0:native, 1:composite
    U8  java:   1;           /// Java method
    U8  immd:   1;           /// Forth immediate word

    U8  data[];              /// name field + parameter field

    char *nfa()         { return (char*)&data[0];  }
    U8   *pfa(U8 off=0) { return &data[len + off]; }
};
#endif // NANOJVM_CORE_H
