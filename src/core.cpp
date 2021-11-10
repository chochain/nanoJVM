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
void Thread::dispatch(IU midx) {
    Word *w  = WORD(midx);
    if (w->java) {                   /// call Java inner interpreter
        IU  addr = *(IU*)w->pfa();
    	java_call(addr);
    }
    else if (w->forth) {			 /// Forth core
        gPool.rs.push(IP);			 /// setup call frame
        gPool.rs.push(WP = midx);
        IP = (IU)(w->pfa() - M0);	 /// get new IP
        while (IP) {				 /// Forth inner interpreter
        	IU midx1 = *(IU*)(M0 + IP); /// fetch next instruction
            LOG("\nm"); LOX4(IP-1); LOG(":"); LOX4(midx1);
            LOG(" "); LOG(WORD(midx1)->nfa());
            IP += sizeof(IU);        /// too bad, we cannot do IP++
            dispatch(midx1);		 /// recursively call
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
    IU cid = fetch2();              /// class index
    char cls[128];
    LOG(" "); LOG(jStrRef(cid, cls));
    IU ci  = gPool.get_class(cls);
    IU oid = gPool.add_obj(ci);
    push(oid);		                /// save object onto stack
}
void Thread::java_call(IU j) {	    /// Java inner interpreter
	U16 nlv = jU16(j - 6);          /// local variable counts, TODO: handle types other than integer
    frame -= nlv;                   /// allocate local variables
    gPool.rs.push(IP);
    IP = j;                         /// pointer to class file
    while (IP) {
        ss_dump(*this);
        U8 op = fetch();            /// fetch JVM opcode
        LOG("j"); LOX4(IP-1); LOG(":"); LOX2(op);
        LOG(" "); LOG(uCode.vt[op].name);
        uCode.exec(*this, op);      /// execute JVM opcode (in microcode ROM)
    }
    IP = gPool.rs.pop();            /// restore stack frame
    frame += nlv;                   /// pop off local variables
}
void Thread::invoke(U16 itype, IU oid) { /// invoke type: 0:virtual, 1:special, 2:static, 3:interface, 4:dynamic
    IU j   = fetch2();              /// 2 - method index in pool
    if (itype>2) IP += 2;           /// extra 2 for interface and dynamic
    IU c_m = jOff(j);               /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cid = jU16(c_m + 1);         /// 12
    IU mid = jU16(c_m + 3);         /// 13
    IU mrf = jOff(mid);             /// [13]008f:c=>[15,16]  [method_name, type_name]

    char cls[128], mn[128], t[16];
    LOG(" "); LOG(jStrRef(cid, cls)); LOG("::"); LOG(jStr(jU16(mrf + 1), mn));
    LOG(jStr(jU16(mrf + 3), t));

    IU ci = gPool.get_class(cls);
    IU mi = gPool.get_method(mn, ci, itype!=1);   /// special does not go up to supr class
    if (mi > 0) dispatch(mi);
    else        LOG(" **NA**");
}
///
/// class and instance variable access
///   Note: use gPool.vref is a bit wasteful but avoid the runtime search. TODO:
///
DU *Thread::cls_var(U16 j) {
    for (int i=0; i<gPool.cv.idx; i++) {
    	if (gPool.cv[i].key == j) {
    		return (DU*)&gPool.pmem[gPool.cv[i].ref];
    	}
    }
    IU c_m = jOff(j);                 /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cid = jU16(c_m + 1);
    char cls[128]; jStrRef(cid, cls); /// get class name
    IU ci = gPool.get_class(cls);     /// map to for class index in pmem
    Word *w = (Word*)&gPool.pmem[ci];
    DU   *d = (DU*)w->pfa(CLS_CV) + gPool.cv.idx;
    gPool.cv.push({ j, (IU)((U8*)d - M0) });
	return d;
}
DU *Thread::inst_var(U16 o, U16 j) {
	Word *obj = (Word*)&gPool.pmem[o];
	for (int i=0; i<gPool.iv.idx; i++) {
    	if (gPool.iv[i].key == j) {
    		return (DU*)obj->pfa() + gPool.iv[i].ref;
    	}
    }
	IU idx = gPool.iv.idx;
    gPool.iv.push({ j, idx });
    return (DU*)obj->pfa() + idx;
}
