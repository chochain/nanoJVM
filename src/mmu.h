#ifndef NANOJVM_MMU_H
#define NANOJVM_MMU_H
#include "core.h"

struct KV {
	IU key;
	IU ref;
	IU nparm;
};
///
/// Memory Pool Manager
/// Note:
///    ucode is fused into vt for now, it can stay in ROM
///
struct Pool {
    List<U8, PMEM_SZ>   pmem;     /// parameter memory
    List<U8, HEAP_SZ>   heap;	  /// object space
    List<DU, RS_SZ>     rs;		  /// global return stack
    ///
    /// JIT lookup tables
    ///
    List<IU, OP_LU_SZ>  op;  	  /// cached opcode->pmem lookup
    List<KV, VT_LU_SZ>  vt;       /// java method lookup
    List<KV, CV_LU_SZ>  cv;       /// class variable lookup
    List<KV, IV_LU_SZ>  iv;       /// instance variable lookup

    template<typename T>
    IU  lookup(T &a, IU j) {
    	for (IU i=0; i<a.idx; i++) if (a[i].key == j) return i;
    	return 0xffff;
    }
    ///
    /// core objects
    ///
    IU jvm_root = 0;              /// JVM methods linked list
    IU cls_root = 0;              /// Class linked list
    IU obj_root = 0;              /// Object linked list

    IU   find(const char *m_name, IU root);
    IU   get_class(const char *cls_name);
    IU   get_method(const char *m_name, IU cls_id=0, bool supr=true);
    ///
    /// dictionary builder
    ///
    IU   add_ucode(Method &vt, IU &root);
    IU   add_method(const char *m_name, IU mj, IU &root);
    IU   add_class(const char *name, const char *supr, IU m_root, U16 cvsz, U16 ivsz);
    void register_class(const char *name, int sz, Method *vt, const char *supr = 0);
    ///
    /// new object instance
    ///
    IU   add_obj(IU cx);
    void obj_u8(U8 b) { heap.push(b); }
    void obj_iu(IU i) { heap.push((U8*)&i, sizeof(IU)); }
    void obj_du(DU v) { heap.push((U8*)&v, sizeof(DU)); }
    ///
    /// compiler methods
    ///
    void build_op_lookup();
    void colon(Thread &t, const char *name);

    void mem_u8(U8 b) { pmem.push(b); }
    void mem_iu(IU i) { pmem.push((U8*)&i, sizeof(IU)); }
    void mem_du(DU v) { pmem.push((U8*)&v, sizeof(DU)); }
    void mem_pu(PU p) { pmem.push((U8*)&p, sizeof(PU)); }
    void mem_str(const char *s) { int sz = STRLEN(s); pmem.push((U8*)s,  sz); }
    void mem_op(U16 i) { mem_iu(op[i]); }
};
extern Pool gPool;
///
/// macros for parameter memory access
///
#define WORD(a)((Word*)&gPool.pmem[a])
#define OBJ(a) ((Word*)&gPool.heap[a])
#define HERE   (gPool.pmem.idx)         /** current parameter memory index           */
#endif // NANOJVM_MMU_H

