#ifndef NANOJVM_MMU_H
#define NANOJVM_MMU_H
#include "core.h"

struct KV {
	IU key;
	IU ref;
};
///
/// Memory Pool Manager
/// Note:
///    ucode is fused into vt for now, it can stay in ROM
///
struct Pool {
    List<U8, PMEM_SZ>   pmem;     /// parameter memory
    List<IU, LOOKUP_SZ> lookup;	  /// cached opcode->pmem lookup
    List<DU, RS_SZ>     rs;		  /// global return stack
    List<KV, CV_SZ>     cv;       /// class variable reference
    List<KV, IV_SZ>     iv;       /// instance variable reference

    IU jvm_root = 0;              /// JVM methods linked list
    IU cls_root = 0;              /// Class linked list
    IU obj_root = 0;              /// Object linked list

    IU   find(const char *m_name, IU root);
    IU   get_class(const char *cls_name);
    IU   get_method(const char *m_name, IU cls_id=0, bool supr=true);
    ///
    /// dictionary builder
    ///
    IU   add_method(Method &vt, IU &root);
    IU   add_method(const char *m_name, U32 m_idx, U8 flag, IU &root);
    IU   add_class(const char *name, const char *supr, IU m_root, U16 cvsz, U16 ivsz);
    void register_class(const char *name, int sz, Method *vt, const char *supr = 0);
    ///
    /// new object instance
    ///
    IU   add_obj(IU ci);
    ///
    /// compiler methods
    ///
    void build_lookup();
    void colon(Thread &t, const char *name);

    void add_u8(U8 b) { pmem.push(b); }
    void add_iu(IU i) { pmem.push((U8*)&i, sizeof(IU)); }
    void add_du(DU v) { pmem.push((U8*)&v, sizeof(DU)); }
    void add_pu(PU p) { pmem.push((U8*)&p, sizeof(PU)); }
    void add_str(const char *s) { int sz = STRLEN(s); pmem.push((U8*)s,  sz); }
    void add_op(IU i) { add_iu(lookup[i]); }
};
extern Pool gPool;
///
/// macros for parameter memory access
///
#define WORD(a)((Word*)&gPool.pmem[a])
#define HERE   (gPool.pmem.idx)         /** current parameter memory index           */
#endif // NANOJVM_MMU_H

