#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <string>       // string class
#include <stdlib.h>     // strtol
#include "ucode.h"
#include "jvm.h"

extern Ucode gUcode;
///
/// Java class file constant pool access macros
///
#define jOff(i)      J.offset(i - 1)
#define jU16(a)      J.getU16(a)
#define jStrRef(i,s) J.getStr(i, s, true)
#define jStr(i,s)    J.getStr(i, s, false)
///
/// Thread class implementation
///
void Thread::forth_call(IU w) {
	Word *m = (Word*)&gPool.pmem[w];
	if (m->def) forth_inner(w);
	else {
		fop xt = *(fop*)m->pfa();
		xt(*this);
	}
}
void Thread::forth_inner(IU w)    {
    gPool.rs.push(IP);
    gPool.rs.push(WP = w);
    Word *m = (Word*)&gPool.pmem[w];
    IP = (IU)(m->pfa() - M0);
    for (IU w1=*(IU*)(M0 + IP); IP!=0; IP += sizeof(IU)) {
        forth_call(w1);
    }
    WP = (IU)gPool.rs.pop();
    IP = gPool.rs.pop();
}
void Thread::java_new()  {
    IU idx = fetch2();              /// class index
    /// TODO: allocate space for the object instance
    char buf[128];
    printf(" %s", jStrRef(idx, buf));
    push(idx);                      /// save object on stack
}
void Thread::java_call(IU addr) {	/// Java inner interpreter
    U16 nloc = jU16(addr - 6);      /// TODO: allocate local stack frame
    gPool.rs.push(IP);
    IP = addr;                      /// pointer to class file
    while (IP) {
        ss_dump();
        U8 op = fetch();            /// fetch opcode
        printf("%04x:%02x %s", IP-1, op, gUcode.vt[op].name);
        gUcode.exec(*this, op);     /// execute JVM opcode
    }
    IP = gPool.rs.pop();
}
void Thread::invoke(U16 itype) {    /// invoke type: 0:virtual, 1:special, 2:static, 3:interface, 4:dynamic
    IU idx   = fetch2();            /// 2 - method index in pool
    if (itype==4) IP += 2;          /// extra 2 for dynamic
    IU c_m   = jOff(idx);           /// [02]000f:a=>[12,13]  [class_name, method_ref]
    IU cid   = jU16(c_m + 1);       /// 12
    IU mrf   = jU16(c_m + 3);       /// 13
    IU mid   = jOff(mrf);           /// [13]008f:c=>[15,16]  [method_name, type_name]

    char cls[128], xt[128], t[16];
    printf(" %s::%s", jStrRef(cid, cls), jStr(jU16(mid + 1), xt));
    printf("%s", jStr(jU16(mid + 3), t));

    IU m = gPool.get_method(xt, cls, itype!=1); /// special does not go up to supr class
    if (m > 0) {
        Word *w  = (Word*)&gPool.pmem[m];
        IU  addr = *(IU*)w->pfa();
        if (w->java) java_call(addr);           /// call Java inner interpreter
        else         forth_call(m);             /// call Native method
    }
    else printf(" **NA**");
}
