#include "mmu.h"

Pool gPool;             /// global memory pool manager
///
/// search for word following given linked list
///
IU Pool::get_parm_idx(const char *parm) {
	if (!parm) return DATA_NA;
	IU pidx = find(parm, parm_root);
	if (pidx != DATA_NA) return pidx;  /// no cached entry found

    IU px = pmem.idx;                  /// store current param list index
    mem_iu(parm_root);                 /// link to previous method
    mem_u8(STRLEN(parm));              /// param list description length
    mem_u8(0);                         /// no access control
    mem_str(parm);                     /// inscribe param list description

    return parm_root = px;             /// adjust method root
}
IU Pool::find(const char *name, IU root, IU parm) {
	if (root==DATA_NA) return DATA_NA; /// no entry yet
    IU idx = root;
    U8 len = STRLEN(name);             /// get length first, speed up matching
    do {
        Word *w = (Word*)&pmem[idx];
        if (w->len==len && strcmp(w->nfa(), name)==0) {
        	if (parm==DATA_NA || !w->java) return idx;
        	if (parm==*(IU*)w->pfa(PFA_PARM_IDX)) return idx;
        }
        idx = w->lfa;
    } while (idx != DATA_NA);
    return idx;
}
///
/// return cls_obj if cls_name is NULL
///
IU Pool::get_class(const char *cls_name) {
    return cls_name ? find(cls_name, cls_root) : jvm_root;
}
///
/// return m_root if m_name is NULL
///
IU Pool::get_method(const char *m_name, IU cls_id, IU parm, bool supr) {
    Word *cls = (Word*)&pmem[cls_id != DATA_NA ? cls_id : cls_root];
    IU mx = DATA_NA;
    while (cls) {
        mx = find(m_name, *(IU*)cls->pfa(PFA_CLS_VT), parm);
        if (mx != DATA_NA || !supr) break;
        cls = (cls->lfa == DATA_NA) ? 0 : (Word*)&pmem[cls->lfa];
    }
    return mx;
}
///
/// method constructor
/// Format:
///     | 16b  |8b  | 8b | len  | 64/32b | 16b  |
///     | LFA  |len |flag| name | xt     | parm |
///
IU Pool::add_ucode(const Method &vt, IU &m_root) {
    IU mx = pmem.idx;               /// store current method index
    mem_iu(m_root);                 /// link to previous method
    mem_u8(STRLEN(vt.name));        /// method name length
    mem_u8((U8)vt.flag);            /// method access control
    mem_str(vt.name);               /// inscribe method name
    mem_pu((PU)vt.xt);              /// encode function pointer
    mem_iu(vt.parm);                /// parameter list
    return m_root = mx;             /// adjust method root
};
IU Pool::add_method(const char *m_name, IU &m_root, IU mjdx, IU parm) {
    IU mx = pmem.idx;               /// store current method index
    mem_iu(m_root);                 /// link to previous method
    mem_u8(STRLEN(m_name));         /// method name length
    mem_u8(FLAG_JAVA);              /// method access control
    mem_str(m_name);                /// inscribe method name
    mem_pu((PU)mjdx);               /// encode function pointer
    mem_iu(parm);                   /// encode parameter list index
    return m_root = mx;             /// adjust method root
};
IU Pool::add_class(const char *name, IU m_root, const char *supr, U16 cvsz, U16 ivsz) {
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
    return cls_root = cx;
}
///
/// class constructor
///
void Pool::register_class(const char *name, const Method *vt, int vtsz, const char *supr, U16 cvsz, U16 ivsz) {
    /// encode vtable
    IU m_root = DATA_NA;
    for (int i=0; i<vtsz; i++) {
        add_ucode(vt[i], m_root);
    }
    if (vtsz) add_class(name, m_root, supr, cvsz, ivsz);
}
///
/// new object instance
///
IU Pool::add_obj(IU cx) {
    Word *w   = (Word*)&pmem[cx];	/// get object class pointer
    U16  ivsz = *(U16*)w->pfa(PFA_CLS_IVSZ);

    if (heap.idx==0) obj_du(0);		/// reserve a null header
    IU oid  = heap.idx;             /// keep object index
    obj_iu(obj_root);				/// add object onto linked list
    obj_u8(cx);                     /// class reference
    obj_u8(0);						/// reserved(for garbage collector)
    obj_allot(ivsz);
    return obj_root = oid;			/// link to previous object and return object id
}
///
/// new Array storage
///
IU Pool::add_array(U8 atype, IU n) {///
	IU oid  = heap.idx;             /// keep object index
    obj_iu(obj_root);				/// add array onto linked list
    obj_u8(n & 0xff);               /// array length (max 64K)
    obj_u8(n >> 8);                 ///
    obj_allot(n);
    return obj_root = oid;
}

void Pool::build_op_lookup() {
	static const char *wlist[OP_LU_SZ] = {
		"dovar", "dolit", "dostr", "unnest"
	};
	IU   mx   = find("ej32/Forth", cls_root);
	Word *cls = (Word*)&pmem[mx];
	IU   vt   = *(IU*)cls->pfa(PFA_CLS_VT);
	for (int i=0; i<OP_LU_SZ; i++) {
		IU w  = find(wlist[i], vt);
		op[i] = w;
	}
}
///
/// word constructor
///
void Pool::colon(IU cls, const char *name) {
    Word *w  = (Word*)&pmem[cls];	    /// get class word
    IU   *vt = (IU*)w->pfa(PFA_CLS_VT); /// pointer to method root
    int  mx  = pmem.idx;			    /// keep current
    mem_iu(*vt);
    mem_u8(STRLEN(name));
    mem_u8(FLAG_FORTH);				    /// a Forth word
    mem_str(name);
    *vt = mx;
}
