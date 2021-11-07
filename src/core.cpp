#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <string>       // string class
#include <stdlib.h>     // strtol
#include "ucode.h"
#include "mmu.h"

extern Ucode gUcode;
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
    IU idx = fetch2();              /// class index
    /// TODO: allocate space for the object instance
    char buf[128];
    LOG(" "); LOG(jStrRef(idx, buf));
    push(idx);                      /// save object on stack
}
void Thread::java_call(IU jidx) {	/// Java inner interpreter
	U16 nlv = jU16(jidx - 6);       /// local variable counts, TODO: handle types other than integer
    frame -= nlv;                   /// allocate local variables
    gPool.rs.push(IP);
    IP = jidx;                      /// pointer to class file
    while (IP) {
        ss_dump(*this);
        U8 op = fetch();            /// fetch opcode
        LOG("j"); LOX4(IP-1); LOG(":"); LOX2(op);
        LOG(" "); LOG(gUcode.vt[op].name);
        gUcode.exec(*this, op);     /// execute JVM opcode
    }
    IP = gPool.rs.pop();            /// restore stack frame
    frame += nlv;                   /// pop off local variables
}
void Thread::invoke(U16 itype, IU jidx) { /// invoke type: 0:virtual, 1:special, 2:static, 3:interface, 4:dynamic
    IU idx   = fetch2();            /// 2 - method index in pool
    if (itype>2) IP += 2;           /// extra 2 for interface and dynamic
    IU c_m   = jOff(idx);           /// [02]000f:a=>[12,13]  [class_name, method_ref]
    IU cid   = jU16(c_m + 1);       /// 12
    IU mrf   = jU16(c_m + 3);       /// 13
    IU midx  = jOff(mrf);           /// [13]008f:c=>[15,16]  [method_name, type_name]

    char cls[128], xt[128], t[16];
    LOG(" "); LOG(jStrRef(cid, cls)); LOG("::"); LOG(jStr(jU16(midx + 1), xt));
    LOG(jStr(jU16(midx + 3), t));

    IU c = gPool.get_class(cls);
    IU m = gPool.get_method(xt, c, itype!=1); /// special does not go up to supr class
    if (m > 0) dispatch(m);
    else       LOG(" **NA**");
}
