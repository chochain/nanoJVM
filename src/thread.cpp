#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <string>       // string class
#include "ucode.h"

extern Ucode uCode;
extern void  ss_dump(Thread &t);
///==========================================================================
/// Thread class implementation
///==========================================================================
///
/// Java class file constant pool access macros
///
#define jOff(j)      J.offset(j - 1)
#define jU16(a)      J.getU16(a)
#define jStrRef(j,s) J.getStr(j, s, true)
#define jStr(j,s)    J.getStr(j, s, false)
#define J16          (wide ? fetch4() : fetch2())
///
/// utilities
///
IU get_nparm(U16 itype, char *parm) {
    char *p = parm+1;
    U16  nparm = itype==2 ? 0 : 1;  /// except static type, all have a object ref
    while (*p != ')') {             /// count number of parameters
        if (*p++=='L') while (*p++ != ';');           /// (Ljava/lang/String;)
        nparm++;
    }
    return nparm;
}
///
/// VM Execution Unit
///
void Thread::na() { LOG(" **NA**"); }/// feature not supported yet
void Thread::dispatch(IU mx, U16 nparm) {
    Word *w = WORD(mx);
    if (w->java) {                   /// call Java inner interpreter
        IU  addr = *(IU*)w->pfa();
        java_call(addr, nparm);
    }
    else if (w->forth) {             /// user defined Forth word
        gPool.rs.push(IP);           /// setup call frame
        IP = (IU)(w->pfa() - M0);    /// get new IP
        while (IP) {                 /// Forth inner interpreter
            mx = *(IU*)(M0 + IP);    /// fetch next instruction
            LOG("\nm"); LOX4(IP-1); LOG(":"); LOX4(mx);
            LOG(" "); LOG(WORD(mx)->nfa());
            IP += sizeof(IU);        /// too bad, we cannot do IP++
            dispatch(mx);            /// recursively call
        }
        IP = gPool.rs.pop();         /// restore call frame
    }
    else {
        fop xt = *(fop*)w->pfa();    /// Native method pointer
        xt(*this);
    }
}
///
/// Java core
///
void Thread::java_new()  {
	IU j = fetch2();                /// class index
	char cls[128];
	LOG(" "); LOG(jStrRef(j, cls));
	IU cx = gPool.get_class(cls);
    IU ox = gPool.add_obj(cx);
    push(ox);                       /// save object onto stack
}
void Thread::java_call(IU j, U16 nparm) {   /// Java inner interpreter
    gPool.rs.push(SP);              /// keep caller stack frame
    SP = ss.idx - nparm + 1;        /// adjust local variable base, extra 1=obj ref, TODO: handle types
    U16 n = ss.idx + jU16(j - 6) - nparm;   /// allocate for local variables
    while (ss.idx < n) push(0);     /// setup local variables, TODO: change ss.idx only

    U8 op;                          /// opcode
    gPool.rs.push(IP);              /// save caller instruction pointer
    IP = j;                         /// pointer to class file
    while (IP) {
        ss_dump(*this);
        op = fetch();               /// fetch JVM opcode
        LOG("j"); LOX4(IP-1); LOG(":"); LOX2(op);
        LOG(" "); LOG(uCode.vt[op].name);
        uCode.exec(*this, op);      /// execute JVM opcode (in microcode ROM)
    }
    IP = gPool.rs.pop();            /// restore to caller IP

    DU rv = op == OP_RETURN ? 0 : pop(); /// check return value

    while (ss.idx >= SP) pop();     /// clean off stack (optional)
    SP = gPool.rs.pop();        	/// restore caller stack frame

    if (op==OP_RETURN) push(rv);    /// add return value if any
}
void Thread::invoke(U16 itype) {    /// invoke type: 0:virtual, 1:special, 2:static, 3:interface, 4:dynamic
    IU j = fetch2();                /// 2 - method index in pool
    if (itype>2) IP += 2;           /// extra 2 for interface and dynamic
    IU mi = gPool.lookup(gPool.vt, j);  /// search cache first
    if (mi != DATA_NA) {
        Word *w = WORD(gPool.vt[mi].ref);
        LOG(" "); LOG(w->nfa());
        dispatch(gPool.vt[mi].ref, gPool.vt[mi].nparm);
        return;
    }
    /// cache missed, create new lookup entry
    IU c_m = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cj  = jU16(c_m + 1);         /// 12
    IU mj  = jU16(c_m + 3);         /// 13
    IU rf  = jOff(mj);              /// [13]008f:c=>[15,16]  [method_name, type_name]

    char cls[128], nm[128], parm[32];
    LOG(" "); LOG(jStrRef(cj, cls));        		 /// get class name
    LOG("."); LOG(jStr(jU16(rf + 1), nm));  		 /// get method name
    LOG(":"); LOG(jStr(jU16(rf + 3), parm));		 /// get param list name

    IU cx = gPool.get_class(cls);                     /// class ref
    IU pi = gPool.get_parm_idx(parm);                 /// parameter list
    IU mx = gPool.get_method(nm, cx, pi, itype!=1);   /// special does not go up to super class
    LOG(" =>$"); LOX(gPool.vt.idx);

    U16 nparm = get_nparm(itype, parm);
    gPool.vt.push({j, mx, nparm});

    if (mx != DATA_NA) dispatch(mx, nparm);
    else               na();
}
///
/// class and instance variable access
///   Note: use gPool.vref is a bit wasteful but avoid the runtime search. TODO:
///
DU *Thread::cls_var() {
	U16 j = J16;
    IU  i = gPool.lookup(gPool.cv, j);
    if (i != DATA_NA) { return (DU*)&gPool.pmem[gPool.cv[i].ref]; }

    /// cache missed, create new lookup entry
    IU c_f = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, field_idx]
    IU cj  = jU16(c_f + 1);
    IU fj  = jU16(c_f + 3);         /// 13
    IU rf  = jOff(fj);              /// [13]008f:c=>[15,16]  [field_name, type_name]

    char cls[128], nm[128], parm[32];
    LOG(" "); LOG(jStrRef(cj, cls));
    LOG("."); LOG(jStr(jU16(rf + 1), nm));
    LOG(":"); LOG(jStr(jU16(rf + 3), parm));

    IU   cx  = gPool.get_class(cls);/// map to for class index in pmem
    Word *w  = WORD(cx);
    DU   *cv = (DU*)w->pfa(PFA_CLS_CV) + gPool.cv.idx;
    LOG(" =>$"); LOX(gPool.cv.idx);

    gPool.cv.push({ j, (IU)((U8*)cv - M0) });  /// create new cache entry
    return cv;
}
DU *Thread::inst_var(IU ox) {
	U16 j   = J16;
    DU  *iv = (DU*)OBJ(ox)->pfa();
    IU  i   = gPool.lookup(gPool.iv, j);
    if (i != DATA_NA) return iv + gPool.iv[i].ref;

    // cache missed, create new lookup entry
    IU c_f = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, field_idx]
    IU cj  = jU16(c_f + 1);         /// 12
    IU fj  = jU16(c_f + 3);         /// 13
    IU rf  = jOff(fj);              /// [13]008f:c=>[15,16]  [field_name, type_name]

    char cls[128], nm[128], parm[32];
    LOG(" "); LOG(jStrRef(cj, cls));
    LOG("."); LOG(jStr(jU16(rf + 1), nm));
    LOG(":"); LOG(jStr(jU16(rf + 3), parm));

    IU idx = gPool.iv.idx;
    LOG(" =>$"); LOX(idx);

    gPool.iv.push({ j, idx });        /// create new cache entry
    return iv + idx;
}
///
/// array support
///   Note: use gPool.heap for array storage linked list obj_root
///
void Thread::java_newa(IU n) {
	U8 j  = fetch();                /// fetch atype value
    if (j != 0xa) na();             /// support only integer, TODO: more types
    else {
        IU ax = gPool.add_array(j, n);
        push(ax);
    }
}
void Thread::java_anewa(IU n) {
	U16 j   = fetch2();             /// fetch 2-dim atype, ignore now, TODO: check type
    IU  c_f = jOff(j);              /// [02]000f:a=>[12,13]  [class_idx, field_idx]
    IU  t2  = jU16(c_f);
    IU  ax  = gPool.add_array(t2 >> 8, n);   /// Note: using DU for ref (IU) is a bit wasteful, but...
    push(ax);
}
IU   Thread::alen(IU ax) {
    IU *p = (IU*)OBJ(ax);
    return *(p + 1);
}
void Thread::astore(IU ax, IU idx, DU v) {
    DU *a0 = (DU*)OBJ(ax)->data;
    *(a0 + idx) = v;
}
DU *Thread::aload(IU ax, IU idx) {
    DU *a0 = (DU*)OBJ(ax)->data;
    return a0 + idx;
}
