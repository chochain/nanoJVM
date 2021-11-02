#ifndef NANOJVM_CORE_H
#define NANOJVM_CORE_H
#include "common.h"         /// common types and configuration
#include "loader.h"         /// Java class loader
///
/// Thread class
///
struct Thread {
    List<DU, SS_SZ>  ss;    /// data stack
    DU     xs[SS_SZ];       /// DEBUG: execution local stack, REFACTOR: combine with ss
    DU     gl[16];          /// DEBUG: class variable (static)
    Loader &J;              /// Java class loader
    U8     *M0    = NULL;   /// cached base address of memory pool

    U16   frame   = 0;      /// local stack index
    bool  compile = false;  /// compile flag
    bool  wide    = false;  /// wide flag
    DU    base    = 10;     /// radix
    DU    tos     = -1;     /// top of stack
    IU    WP      = 0;      /// method index
    IU    IP      = 0;      /// instruction pointer (program counter)

    Thread(Loader &ldr, U8 *mem) : J(ldr), M0(mem) {}
    ///
    /// Java class file byte fetcher
    ///
    U8   fetch()        { return J.getU8(IP++); }
    U16  fetch2()       { U16 n = J.getU16(IP); IP+=2; return n; }
    U32  fetch4()       { U32 n = J.getU32(IP); IP+=4; return n; }
    ///
    /// branching ops
    ///
    void ret()          { IP = 0; }
    void jmp()          { IP += J.getU16(IP) - 1; }
    void cjmp(bool f)   { IP += f ? J.getU16(IP) - 1 : sizeof(U16); }
    ///
    /// stack ops
    ///
    void push(DU v)     { ss.push(tos); tos = v; }
    DU   pop()          { DU n = tos; tos = ss.pop(); return n; }
    ///
    /// Java class/method ops
    ///
    void java_new();
    void java_inner(IU midx); /// execute Java method
    void invoke(U16 itype);
    ///
    /// local parameter access, CC:TODO
    ///
    template<typename T>
    T    load(U32 i, T n)  { return *(T*)&xs[frame+i]; }
    template<typename T>
    void store(U32 i, T n) { *(T*)&xs[frame+i] = n; }
};
typedef void (*fop)(Thread&); /// opcode function pointer
///
/// Method class
///
#define FLAG_IMMD 0x1
#define FLAG_DEF  0x2
#define FLAG_JAVA 0x4
struct Method {
    const char *name = 0;     /// for debugging, TODO (in const_pool)
#if METHOD_PACKED
    union {
        fop   xt = 0;         /// function pointer (or decayed lambda)
        struct {
            U16 flag;
            IU  midx;
        };
    };
#else
    fop   xt = 0;            /// function pointer (or decayed lambda)
    U16   flag;
    IU    midx;
#endif
    Method(const char *n, fop f, U32 im=0) : name(n), xt(f), flag(im) {}
};
///
/// Word - shared struct for Class and Method
///   class list - linked list of words, dict[cls_root], pfa => next_class
///   vtable     - linked list of words, dict[class.pfa], pfa => next_method
///
#define CLS_SUPER       0
#define CLS_INTF        2
#define CLS_VT          4
#define CLS_CVSZ        6
#define CLS_IVSZ        8
struct Word {                /// 4-byte header
    IU  lfa;                 /// link field to previous word
    U8  len;                 /// name of method

    U8  def:    1;           /// 0:native, 1:composite
    U8  immd:   1;           /// Forth immediate word
    U8  java:   1;           /// Java method
    U8  access: 2;           /// public, private, protected
    U8  ftype:  3;           /// static, finall, virtual, synchronized

    U8  data[];              /// name field + parameter field

    char *nfa()         { return (char*)&data[0];  }
    U8   *pfa(U8 off=0) { return &data[len + off]; }
};
#endif // NANOJVM_CORE_H
