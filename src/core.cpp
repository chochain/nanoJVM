#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <string>       // string class
#include <stdlib.h>     // strtol
#include "ucode.h"
#include "mmu.h"

extern Ucode uCode;
extern void  ss_dump(Thread &t);

///==========================================================================
/// Thread class implementation
///==========================================================================
///
/// Java class file constant pool access macros
///
#define jOff(i)      J.offset(i - 1)
#define jU16(a)      J.getU16(a)
#define jStrRef(i,s) J.getStr(i, s, true)
#define jStr(i,s)    J.getStr(i, s, false)
///
/// VM Execution Unit
///
void Thread::dispatch(IU mx, U16 nparm) {
    Word *w = WORD(mx);
    if (w->java) {                   /// call Java inner interpreter
        IU  addr = *(IU*)w->pfa();
    	java_call(addr, nparm);
    }
    else if (w->forth) {			 /// Forth core
        gPool.rs.push(IP);			 /// setup call frame
        gPool.rs.push(WP = mx);
        IP = (IU)(w->pfa() - M0);	 /// get new IP
        while (IP) {				 /// Forth inner interpreter
        	mx = *(IU*)(M0 + IP);    /// fetch next instruction
            LOG("\nm"); LOX4(IP-1); LOG(":"); LOX4(mx);
            LOG(" "); LOG(WORD(mx)->nfa());
            IP += sizeof(IU);        /// too bad, we cannot do IP++
            dispatch(mx);		     /// recursively call
        }
        WP = (IU)gPool.rs.pop();     /// restore call frame
        IP = gPool.rs.pop();
    }
    else {
		fop xt = *(fop*)w->pfa();	 /// Native method pointer
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
    push(ox);		                /// save object onto stack
}
void Thread::java_call(IU j, U16 nparm) {	/// Java inner interpreter
	gPool.rs.push(SP);				/// keep caller stack frame
	SP = ss.idx - nparm + 1;        /// adjust local variable base, extra 1=obj ref, TODO: handle types
	U16 n = ss.idx + jU16(j - 6) - nparm;   /// allocate for local variables
	while (ss.idx < n) push(0);	    /// setup local variables, TODO: change ss.idx only
    gPool.rs.push(IP);              /// save caller instruction pointer
    IP = j;                         /// pointer to class file
    while (IP) {
        ss_dump(*this);
        U8 op = fetch();            /// fetch JVM opcode
        LOG("j"); LOX4(IP-1); LOG(":"); LOX2(op);
        LOG(" "); LOG(uCode.vt[op].name);
        uCode.exec(*this, op);      /// execute JVM opcode (in microcode ROM)
    }
    IP = gPool.rs.pop();            /// restore to caller IP
    while (ss.idx >= SP) pop();	    /// clean up parameters, TODO: change ss.idx only
    SP = gPool.rs.pop();            /// restore caller stack frame
}
void Thread::invoke(U16 itype) {    /// invoke type: 0:virtual, 1:special, 2:static, 3:interface, 4:dynamic
    IU j = fetch2();                /// 2 - method index in pool
    if (itype>2) IP += 2;           /// extra 2 for interface and dynamic
    IU mi = gPool.lookup(gPool.vt, j);
    if (mi != 0xffff) {
    	Word *w = WORD(gPool.vt[mi].ref);
    	LOG(" "); LOG(w->nfa());
    	dispatch(gPool.vt[mi].ref, gPool.vt[mi].nparm);
    	return;
    }
    /// cache missed, create new lookup entry
    IU c_m = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cj  = jU16(c_m + 1);         /// 12
    IU mj  = jU16(c_m + 3);         /// 13
    IU mrf = jOff(mj);              /// [13]008f:c=>[15,16]  [method_name, type_name]

    char cls[128], mn[128], t[16];
    LOG(" "); LOG(jStrRef(cj, cls)); LOG("::"); LOG(jStr(jU16(mrf + 1), mn));
    LOG(jStr(jU16(mrf + 3), t));
    char *p = t+1;
    U16  nparm = itype==2 ? 0 : 1;
    while (*p++ != ')') nparm++;

    IU cx = gPool.get_class(cls);					/// class ref
    IU mx = gPool.get_method(mn, cx, itype!=1);   	/// special does not go up to supr class
    LOG(" $"); LOX(gPool.vt.idx);
    gPool.vt.push({j, mx, nparm});
    if (mx > 0) dispatch(mx, nparm);
    else        LOG(" **NA**");
}
///
/// class and instance variable access
///   Note: use gPool.vref is a bit wasteful but avoid the runtime search. TODO:
///
DU *Thread::cls_var(U16 j) {
	IU i = gPool.lookup(gPool.cv, j);
	if (i != 0xffff) { return (DU*)&gPool.pmem[gPool.cv[i].ref]; }
    /// cache missed, create new lookup entry
    IU c_f = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cj  = jU16(c_f + 1);
    char cls[128]; jStrRef(cj, cls);/// get class name
    IU cx  = gPool.get_class(cls);  /// map to for class index in pmem

#if ENABLE_DEBUG
    IU fj  = jU16(c_f + 3);         /// 13
    IU frf = jOff(fj);              /// [13]008f:c=>[15,16]  [method_name, type_name]
    char fn[128], t[16];
    LOG(" "); LOG(cls); LOG("::"); LOG(jStr(jU16(frf + 1), fn));
    LOG(jStr(jU16(frf + 3), t));
#endif // ENABLE_DEBUG

    Word *w  = WORD(cx);
    DU   *cv = (DU*)w->pfa(CLS_CV) + gPool.cv.idx;
    LOG(" $"); LOX(gPool.cv.idx);
    gPool.cv.push({ j, (IU)((U8*)cv - M0) });  /// create new cache entry
	return cv;
}
DU *Thread::inst_var(IU ox, U16 j) {
	DU *iv = (DU*)OBJ(ox)->pfa();
	IU i   = gPool.lookup(gPool.iv, j);
	if (i != 0xffff) return iv + gPool.iv[i].ref;

	// cache missed, create new lookup entry
#if ENABLE_DEBUG
    IU c_f = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cj  = jU16(c_f + 1);         /// 12
    IU fj  = jU16(c_f + 3);         /// 13
    IU frf = jOff(fj);              /// [13]008f:c=>[15,16]  [method_name, type_name]
    char cls[128], fn[128], t[16];
    LOG(" "); LOG(jStrRef(cj, cls)); LOG("::"); LOG(jStr(jU16(frf + 1), fn));
    LOG(jStr(jU16(frf + 3), t));
#endif // ENABLE_DEBUG

    IU idx = gPool.iv.idx;
    LOG(" $"); LOX(idx);
    gPool.iv.push({ j, idx });        /// create new cache entry
    return iv + idx;
}
