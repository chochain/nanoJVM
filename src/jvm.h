#ifndef NANOJVM_JVM_H
#define NANOJVM_JVM_H
#include "core.h"

enum { DOVAR = 0xCA, DOLIT, DOSTR, UNNEST };    /// Forth opcodes
///
/// VM namespace functions
///
char *next_word();
char *scan(char c);
void words();
void ss_dump();
void mem_dump(IU p0, DU sz);
///
/// macros for parameter memory access
///
#define STR(a) ((char*)&dict[a])  /** fetch string pointer to parameter memory */
#define HERE   (pmem.idx)         /** current parameter memory index           */
///
/// Memory Pool Manager
/// Note:
///    ucode is fused into vt for now, it can stay in ROM
///
struct Pool {
    List<U8, PMEM_SZ>  pmem;      /// parameter memory
    List<DU, CONST_SZ> cnst;      /// constant pool
    IU jvm_root = 0;              /// JVM builtin opcodes
    IU cls_root = 0;              /// Forth::context

    IU   find(const char *m_name, IU root);
    IU   add_method(Method &vt, IU &root);
    IU   get_class(const char *cls_name);
    IU   get_method(const char *m_name, const char *cls_name=0, bool supr=true);

    IU   register_class(const char *name, int sz, Method *vt, const char *supr = 0);

    template <typename T>
    int add_const(T *v) { return cnst.push(*(DU*)v); }
    ///
    /// compiler methods
    ///
    void colon(const char *name);
    void add_u8(U8 b) { pmem.push(b); }
    void add_iu(IU i) { pmem.push((U8*)&i, sizeof(IU)); }
    void add_du(DU v) { pmem.push((U8*)&v, sizeof(DU)); }
    void add_pu(PU p) { pmem.push((U8*)&p, sizeof(PU)); }
    void add_str(const char *s) { int sz = STRLEN(s); pmem.push((U8*)s,  sz); }
};
extern Pool gPool;

#endif // NANOJVM_JVM_H

