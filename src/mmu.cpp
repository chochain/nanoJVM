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
IU Pool::find(const char *name, IU root, IU pidx) {
	if (root==DATA_NA) return DATA_NA; /// no entry yet
    IU idx = root;
    U8 len = STRLEN(name);             /// get length first, speed up matching
    do {
        Word *w = (Word*)&pmem[idx];
        if (w->len==len && strcmp(w->nfa(), name)==0) {
        	if (pidx==DATA_NA || (w->access==ACL_BUILTIN)) return idx;
        	if (pidx==*(IU*)w->pfa(PFA_PARM_IDX)) return idx;
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
IU Pool::get_method(const char *m_name, IU cls_id, IU pidx, bool supr) {
    Word *cls = (Word*)&pmem[cls_id != DATA_NA ? cls_id : cls_root];
    IU mx = DATA_NA;
    while (cls) {
        mx = find(m_name, *(IU*)cls->pfa(PFA_CLS_VT), pidx);
        if (mx != DATA_NA || !supr) break;
        cls = (cls->lfa == DATA_NA) ? 0 : (Word*)&pmem[cls->lfa];
    }
    return mx;
}
///
/// method, class constructor
///
///   Word Memory Format: shared between ucode, method, and class
///     |   word hdr     | str  |
///     | 16b  |8b  | 8b | len  | 64/32b | 16b  |
///     | LFA  |len |flag| name | xt     | parm |
IU Pool::mem_hdr(IU &root, const char *nf, U8 flag) {
	IU rx = pmem.idx;              /// capture current memory index
	mem_iu(root);                  /// link to previous method
	mem_u8(STRLEN(nf));            /// method name length
	mem_u8(flag);                  /// method access control
	mem_str(nf);                   /// inscribe method name
	return root = rx;              /// adjust linked list root
}
IU Pool::add_ucode(IU &m_root, const Method &vt, IU pidx) {
    mem_hdr(m_root, vt.name, vt.flag);
	mem_pu((PU)vt.xt);             /// encode function pointer
	mem_iu(pidx);                  /// parameter list index
    return m_root;
};
IU Pool::add_method(IU &m_root, const char *m_name, IU mjdx, IU pidx) {
    mem_hdr(m_root, m_name, JAVA_FUNC);
	mem_pu((PU)mjdx);              /// encode function pointer
	mem_iu(pidx);                  /// parameter list index
    return m_root;
};
IU Pool::add_class(const char *c_name, IU m_root, const char *supr, U16 cvsz, U16 ivsz) {
	mem_hdr(cls_root, c_name, 0); /// create class header
	mem_iu(get_class(supr));       /// encode super class idx
	mem_iu(0);                     /// reserved field
	mem_iu(m_root);                /// encode class vtable
    mem_iu(cvsz);                  /// cvsz - class variable size
    mem_iu(ivsz);                  /// ivsz - instance variable size
    for (int i=0; i<cvsz; i+=sizeof(DU)) {	/// allocate static variables
    	mem_du(0);
    }
    return cls_root;
}
///
/// class constructor
///
void Pool::register_class(const char *name, const Method *vt, int vtsz, const char *supr, U16 cvsz, U16 ivsz) {
    /// encode vtable
    IU m_root = DATA_NA;
    for (int i=0; i<vtsz; i++) {
    	IU pidx = get_parm_idx(vt[i].parm);  /// cache parameter list string
        add_ucode(m_root, vt[i], pidx);      /// create microcode, TODO: with ROM
    }
    if (vtsz) add_class(name, m_root, supr, cvsz, ivsz);
}
///
/// new object instance
///
IU Pool::obj_hdr(IU n, U16 sz) {
	IU oid  = heap.idx;             /// keep object index
    obj_iu(obj_root);				/// encode object linked list root
    obj_iu(n);                      /// encode value
    obj_allot(sz);
    return obj_root = oid;
}
IU Pool::add_obj(IU cx) {
    Word *w   = (Word*)&pmem[cx];	/// get object class pointer
    U16  ivsz = *(U16*)w->pfa(PFA_CLS_IVSZ);
    return obj_hdr(cx, ivsz);       /// encode class reference with ivsz allocation
}
///
/// new Array storage
/// Note: atype is ignored for now, TODO:
///
IU Pool::add_array(U8 atype, IU n) {
	return obj_hdr(n, sizeof(DU) * n);  /// allocate array w length (max 64K)
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
    mem_hdr(*vt, name, FORTH_FUNC);    /// create word header
}
