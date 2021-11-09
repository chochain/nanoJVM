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
/// class and instance variable access
///   Note: TODO: use global variable for now
///
DU *Thread::cls_var(U16 j) {
	return &gl[j];
}
DU *Thread::inst_var(U16 j) {
	return &gl[j];
}
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
    IU jdx = fetch2();              /// class index
    char buf[128];
    LOG(" "); LOG(jStrRef(jdx, buf));
    /// TODO: allocate space for the object instance
    push(jdx);                      /// save object on stack, fake now
}
void Thread::java_call(IU jdx) {	/// Java inner interpreter
	U16 nlv = jU16(jdx - 6);        /// local variable counts, TODO: handle types other than integer
    frame -= nlv;                   /// allocate local variables
    gPool.rs.push(IP);
    IP = jdx;                       /// pointer to class file
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
    IU jdx   = fetch2();            /// 2 - method index in pool
    if (itype>2) IP += 2;           /// extra 2 for interface and dynamic
    IU c_m   = jOff(jdx);           /// [02]000f:a=>[12,13]  [class_idx, method_idx]
    IU cid   = jU16(c_m + 1);       /// 12
    IU mid   = jU16(c_m + 3);       /// 13
    IU mrf   = jOff(mid);           /// [13]008f:c=>[15,16]  [method_name, type_name]

    char cls[128], mn[128], t[16];
    LOG(" "); LOG(jStrRef(cid, cls)); LOG("::"); LOG(jStr(jU16(mrf + 1), mn));
    LOG(jStr(jU16(mrf + 3), t));

    IU c = gPool.get_class(cls);
    IU m = gPool.get_method(mn, c, itype!=1); /// special does not go up to supr class
    if (m > 0) dispatch(m);
    else       LOG(" **NA**");
}
