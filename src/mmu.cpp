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
    IU m_idx = 0;
    while (cls) {
        m_idx = find(m_name, *(IU*)cls->pfa(CLS_VT));
        if (m_idx || !supr) break;
        cls = cls->lfa ? (Word*)&pmem[cls->lfa] : 0;
    }
    return m_idx;
}
///
/// method constructor
///
IU Pool::add_method(Method &vt, IU &m_root) {
    IU mid = pmem.idx;              /// store current method idx
    add_iu(m_root);                 /// link to previous method
    add_u8(STRLEN(vt.name));        /// method name length
    add_u8((U8)vt.flag);            /// method access control
    add_str(vt.name);               /// enscribe method name
    add_pu((PU)vt.xt);              /// encode function pointer
    return m_root = mid;            /// adjust method root
};
IU Pool::add_method(const char *m_name, U32 m_idx, U8 flag, IU &m_root) {
    IU mid = pmem.idx;              /// store current method idx
    add_iu(m_root);                 /// link to previous method
    add_u8(STRLEN(m_name));         /// method name length
    add_u8(flag);                   /// method access control
    add_str(m_name);                /// enscribe method name
    add_du((DU)m_idx);              /// encode function pointer
    return m_root = mid;            /// adjust method root
};
IU Pool::add_class(const char *name, const char *supr, IU m_root, U16 cvsz, U16 ivsz) {
    /// encode class
    IU cid = pmem.idx;              /// preserve class link
    add_iu(cls_root);               /// class linked list
    add_u8(STRLEN(name));           /// class name string length
    add_u8(0);                      /// public
    add_str(name);                  /// class name string
    add_iu(get_class(supr));        /// super class
    add_iu(0);                      /// interface
    add_iu(m_root);                 /// vt
    add_iu(cvsz);                   /// cvsz
    add_iu(cvsz);                   /// ivsz
    for (int i=0; i<cvsz; i+=sizeof(DU)) {	/// allocate static variables
    	add_du(0);
    }
    if (!jvm_root) jvm_root = cid;  /// mark JVM ucode root
    return cls_root = cid;
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

void Pool::build_lookup() {
	static const char *wlist[LOOKUP_SZ] = {
		"dovar", "dolit", "dostr", "unnest"
	};
	Word *cls = WORD(find("ej32/Forth", cls_root));
	IU   vt   = *(IU*)cls->pfa(CLS_VT);
	for (int i=0; i<LOOKUP_SZ; i++) {
		IU w = find(wlist[i], vt);
		lookup[i] = w;
	}
}
///
/// word constructor
///
void Pool::colon(Thread &t, const char *name) {
    Word *w  = (Word*)&pmem[t.cls];	/// get class word
    IU   *vt = (IU*)w->pfa(CLS_VT); /// pointer to method root
    int  mid = pmem.idx;			/// keep current
    add_iu(*vt);
    add_u8(STRLEN(name));
    add_u8(FLAG_FORTH);				/// a Forth word
    add_str(name);
    *vt = mid;
}
