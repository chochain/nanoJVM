#include "common.h"
#include "mmu.h"

Pool gPool;             /// global memory pool manager
///
/// search for word following given linked list
///
IU Pool::find(const char *s, IU idx) {
    U8 len = STRLEN(s); // check length first, speed up matching
    do {
        Word *w = (Word*)&pmem[idx];
        if (w->len==len && strcmp(w->nfa(), s)==0) return idx;
        idx = w->lfa;
    } while (idx);
    return 0;
}
///
/// return cls_obj if cls_name is NULL
///
IU Pool::get_class(const char *cls_name) {
    return cls_name ? find(cls_name, cls_root) : cls_root;
}
///
/// return m_root if m_name is NULL
///
IU Pool::get_method(const char *m_name, IU cls_id, bool supr) {
    Word *cls = (Word*)&pmem[cls_id ? cls_id : cls_root];
    IU mx = 0;
    while (cls) {
        mx = find(m_name, *(IU*)cls->pfa(CLS_VT));
        if (mx || !supr) break;
        cls = cls->lfa ? (Word*)&pmem[cls->lfa] : 0;
    }
    return mx;
}
///
/// method constructor
///
IU Pool::add_method(Method &vt, IU &m_root) {
    IU mx = pmem.idx;               /// store current method idx
    mem_iu(m_root);                 /// link to previous method
    mem_u8(STRLEN(vt.name));        /// method name length
    mem_u8((U8)vt.flag);            /// method access control
    mem_str(vt.name);               /// enscribe method name
    mem_pu((PU)vt.xt);              /// encode function pointer
    return m_root = mx;             /// adjust method root
};
IU Pool::add_method(const char *m_name, U32 m_idx, U8 flag, IU &m_root) {
    IU mx = pmem.idx;               /// store current method idx
    mem_iu(m_root);                 /// link to previous method
    mem_u8(STRLEN(m_name));         /// method name length
    mem_u8(flag);                   /// method access control
    mem_str(m_name);                /// enscribe method name
    mem_du((DU)m_idx);              /// encode function pointer
    return m_root = mx;             /// adjust method root
};
IU Pool::add_class(const char *name, const char *supr, IU m_root, U16 cvsz, U16 ivsz) {
    /// encode class
    IU cx = pmem.idx;               /// preserve class link
    mem_iu(cls_root);               /// class linked list
    mem_u8(STRLEN(name));           /// class name string length
    mem_u8(0);                      /// public
    mem_str(name);                  /// class name string
    mem_iu(get_class(supr));        /// super class
    mem_iu(0);                      /// interface
    mem_iu(m_root);                 /// vt
    mem_iu(cvsz);                   /// cvsz
    mem_iu(ivsz);                   /// ivsz
    for (int i=0; i<cvsz; i+=sizeof(DU)) {	/// allocate static variables
    	mem_du(0);
    }
    if (!jvm_root) jvm_root = cx;  /// mark JVM ucode root
    return cls_root = cx;
}
///
/// class contructor
///
void Pool::register_class(const char *name, int sz, Method *vt, const char *supr) {
    /// encode vtable
    IU m_root = 0;
    for (int i=0; i<sz; i++) {
        add_method(vt[i], m_root);
    }
    if (sz) add_class(name, supr, m_root, 0, 0);
}
///
/// new object instance
///
IU Pool::add_obj(IU cx) {
    Word *w   = (Word*)&pmem[cx];	/// get object class pointer
    U16  ivsz = *(U16*)w->pfa(CLS_IVSZ);
    IU   oid  = heap.idx;
    obj_iu(obj_root);				/// add object onto linked list
    obj_iu(0);						/// reserved(for garbage collector)
    for (int i=0; i<ivsz; i+=sizeof(DU)) {
    	obj_du(0);
    }
    return obj_root = oid;			/// return object id
}

void Pool::build_op_lookup() {
	static const char *wlist[OP_LU_SZ] = {
		"dovar", "dolit", "dostr", "unnest"
	};
	IU   mx   = find("ej32/Forth", cls_root);
	Word *cls = (Word*)&pmem[mx];
	IU   vt   = *(IU*)cls->pfa(CLS_VT);
	for (int i=0; i<OP_LU_SZ; i++) {
		IU w = find(wlist[i], vt);
		op[i] = w;
	}
}
///
/// word constructor
///
void Pool::colon(Thread &t, const char *name) {
    Word *w  = (Word*)&pmem[t.cls];	/// get class word
    IU   *vt = (IU*)w->pfa(CLS_VT); /// pointer to method root
    int  mx  = pmem.idx;			/// keep current
    mem_iu(*vt);
    mem_u8(STRLEN(name));
    mem_u8(FLAG_FORTH);				/// a Forth word
    mem_str(name);
    *vt = mx;
}
