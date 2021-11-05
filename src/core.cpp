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
    Word *w  = (Word*)&gPool.pmem[midx];
    IU  addr = *(IU*)w->pfa();
    if (w->java) java_call(addr);           /// call Java inner interpreter
    else         forth_call(midx);          /// call Native method
}
///
/// Forth core
///
void Thread::forth_call(IU midx) {
	Word *w = (Word*)&gPool.pmem[midx];
	if (w->def) forth_inner(midx);
	else {
		fop xt = *(fop*)w->pfa();
		xt(*this);
	}
}
void Thread::forth_inner(IU midx)    {
    gPool.rs.push(IP);
    gPool.rs.push(WP = midx);
    Word *w = (Word*)&gPool.pmem[midx];
    IP = (IU)(w->pfa() - M0);
    for (IU m1=*(IU*)(M0 + IP); IP!=0; IP += sizeof(IU)) {
        forth_call(m1);
    }
    WP = (IU)gPool.rs.pop();
    IP = gPool.rs.pop();
}
///
/// Java core
///
void Thread::java_new()  {
    IU idx = fetch2();              /// class index
    /// TODO: allocate space for the object instance
    char buf[128];
    printf(" %s", jStrRef(idx, buf));
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
        printf("%04x:%02x %s", IP-1, op, gUcode.vt[op].name);
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
    printf(" %s::%s", jStrRef(cid, cls), jStr(jU16(midx + 1), xt));
    printf("%s", jStr(jU16(midx + 3), t));

    IU c = gPool.get_class(cls);
    IU m = gPool.get_method(xt, c, itype!=1); /// special does not go up to supr class
    if (m > 0) dispatch(m);
    else       printf(" **NA**");
}
