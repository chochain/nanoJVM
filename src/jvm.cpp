#include <sstream>      // stringstream
#include <iostream>     // cin, cout
#include <iomanip>      // setbase
#include "ucode.h"		// microcode manager
#include "mmu.h"		// memory pool manager
#include "thread.h"     // thread/task interface (include loader.h)
#include "jvm.h"		// virtual machine core

using namespace std;    // default to C++ standard template library
///
/// JVM streaming IO
///
ostringstream   jout;                   /// JVM output stream
void (*jout_cb)(int, const char*);      /// JVM output callback function

#define ENDL    endl; jout_cb(jout.str().length(), jout.str().c_str()); jout.str("")
///
/// Java Virtual Machine implementation
///
extern   Ucode  uCode;                  /// Java microcode ROM
extern   Ucode  uForth;                 /// Forth microcode ROM
extern   Ucode  uESP32;                 /// ESP32 supporting functions
extern   Pool   gPool;                  /// memory pool manager
Loader   gLoader;                       /// loader instance
Thread   gT0(gLoader);  				/// main thread, only one for now
///
/// Java Native IO functions
/// Note: support only string and integer (like Arduino)
///
void _print_s(Thread &t) {
	IU j = t.pop(), ox = t.pop();  /// java constant pool object
	char buf[128];
	jout << " " << t.J.getStr(j, buf, true);
}
void _print_i(Thread &t) {
	IU j = t.pop(), ox = t.pop();  /// java constant pool object
	jout << " " << setbase(t.base) << j;
}
void _println_s(Thread &t) { _print_s(t); jout << ENDL; }
void _println_i(Thread &t) { _print_i(t); jout << ENDL; }
///
/// JVM Core
///
void jvm_to_con(int len, const char *msg) { LOG(msg); }
int  jvm_setup(const char *fname) {
	const static Method uObj[] = {{ "<init>", [](Thread &t){ t.pop(); }, false }};
    const static Method uStr[] = {{ "<init>", [](Thread &t){ t.pop(); }, false }};
	const static Method uSys[] = {{ "<init>", [](Thread &t){ t.pop(); }, false }};
    const static Method uPrs[] = {
    	{ "print",   _print_s,   false, PARM_STR },
    	{ "print",   _print_i,   false, PARM_INT },
    	{ "println", _println_s, false, PARM_STR },
    	{ "println", _println_i, false, PARM_INT }
    };
    setvbuf(stdout, NULL, _IONBF, 0);
    jout_cb = jvm_to_con;
    ///
    /// populate Java classes
    ///
    gPool.register_class("Ucode",              uCode.vt,  uCode.vtsz);
    gPool.register_class("java/lang/Object",   uObj, sizeof(uObj)/sizeof(Method), "Ucode");
    gPool.register_class("java/lang/String",   uStr, sizeof(uStr)/sizeof(Method), "java/lang/Object", sizeof(DU)*3, 0);
    gPool.register_class("java/lang/System",   uSys, sizeof(uSys)/sizeof(Method), "java/lang/Object", sizeof(DU)*3, 0);
    gPool.register_class("java/io/PrintStream",uPrs, sizeof(uPrs)/sizeof(Method), "java/lang/Object");
    gPool.jvm_root = gPool.cls_root;
    ///
    /// Add Forth classes
    ///
    gPool.register_class("ej32/Forth", uForth.vt, uForth.vtsz, "java/lang/Object");
    gPool.register_class("ej32/ESP32", uESP32.vt, uESP32.vtsz, "ej32/Forth");
    gPool.build_op_lookup();
    ///
    /// instantiate Java class loader
    ///
    if (gLoader.init(fname)) return -1;
    
    IU cx = gLoader.load_class();
    if (!cx) return -2;

    gT0.init(&gPool.pmem[0], cx);
    return 0;
}

#if ARDUINO
void mem_stat() {
    LOG("Core:");           LOX(xPortGetCoreID());
    LOG(" heap[maxblk=");   LOX(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    LOG(", avail=");        LOX(heap_caps_get_free_size(MALLOC_CAP_8BIT));
    LOG(", ss_max=");       LOX(t.ss.max);
    LOG(", rs_max=");       LOX(gPool.rs.max);
    LOG(", pmem=");         LOX(gPool.pmem.idx);
    LOG("], lowest[heap="); LOX(heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
    LOG(", stack=");        LOX(uxTaskGetStackHighWaterMark(NULL));
    LOG("]\n");
    if (!heap_caps_check_integrity_all(true)) {
//        heap_trace_dump();     // dump memory, if we have to
        abort();                 // bail, on any memory error
    }
}
String console_cmd;
void jvm_run() {
    if (Serial.available()) {
        console_cmd = Serial.readString();
        LOG(console_cmd);
        outer(gT0, console_cmd.c_str(), send_to_con);
        mem_stat(gT0);
        delay(2);
    }
}
#else 
void mem_stat() {}
void jvm_run() {
    ///
    /// instantiate main thread (TODO: single thread for now)
    ///
    LOG("\nmain()");
    IU mx = gPool.get_method("main", 0);
    
    gT0.dispatch(mx);
}
#endif // ARDUINO

