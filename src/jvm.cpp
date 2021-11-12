#include <sstream>      // stringstream
#include <iostream>     // cin, cout
#include <iomanip>      // setbase
#include <string>       // string class
#include <stdlib.h>     // strtol
#include "ucode.h"		// microcode managemer
#include "mmu.h"		// memory pool manager
#include "jvm.h"		// virtual machine core

using namespace std;    // default to C++ standard template library
///
/// Java Virtual Machine implementation
///
extern   Ucode  uCode;                  /// Java microcode ROM
extern   Ucode  uForth;                 /// Forth microcode ROM
extern   Ucode  uESP32;                 /// ESP32 supporting functions
extern   Pool   gPool;                  /// memory pool manager
Loader   gLoader;                       /// loader instance
Thread   gT0(gLoader);  				/// main thread, only one for now

istringstream   fin;                    /// forth_in
ostringstream   fout;                   /// forth_out
string strbuf;                          /// input string buffer
void (*fout_cb)(int, const char*);      /// forth output callback function

#define ENDL    endl; fout_cb(fout.str().length(), fout.str().c_str()); fout.str("")
///
/// debug helper
///
void words(Thread &t) {
    fout << setbase(16);
    IU cid = gPool.cls_root;
    do {
        Word *cls = WORD(cid);
        Word *supr= WORD(*(IU*)cls->pfa(CLS_SUPR));
        fout << "\n" << cls->nfa() << " : " << supr->nfa() << " " << cid;
        IU mid = *(IU*)cls->pfa(CLS_VT);
        int i = 0;
        do {
            Word *w = WORD(mid);
            if ((i++%10)==0) { fout << ENDL; fout << "\t"; yield(); }
            fout << w->nfa() << " " << mid << " ";
            mid = w->lfa;
        } while (mid);
        cid = cls->lfa;
    } while (cid && cid != gPool.jvm_root);
    fout << setbase(t.base) << ENDL;
}
void ss_dump(Thread &t) {
    if (t.compile) return;
    fout << setbase(t.base) << " <";
    for (int i=0; i<t.ss.idx; i++) {
    	fout << t.ss[i] << ((i+1)==t.SP ? "|" : " ");
    }
    fout << t.tos << "> ok" << ENDL;
    yield();
}
///
/// dump pmem at p0 offset for sz bytes
///
void mem_dump(Thread &t, IU p0, DU sz) {
    fout << setbase(16) << setfill('0') << ENDL;
    char buf[17] = { 0 };
    for (IU i=ALIGN16(p0); i<=ALIGN16(p0+sz); i+=16) {
        fout << "m" << setw(4) << i << ": ";
        for (int j=0; j<16; j++) {
            U8 c = gPool.pmem[i+j];
            buf[j] = ((c>0x7f)||(c<0x20)) ? '_' : c;
            fout << setw(2) << (int)c << (j%4==3 ? "  " : " ");
        }
        fout << buf << ENDL;
        yield();
    }
    fout << setbase(t.base);
}
///
/// outer interpreter
///
char *next_word()  {
    fin >> strbuf;
    return (char*)strbuf.c_str();
}
char *scan(char c) {
    getline(fin, strbuf, c);
    return (char*)strbuf.c_str();
}
int handle_number(Thread &t, const char *idiom) {
    char *p;
    int n = static_cast<int>(strtol(idiom, &p, t.base));
    LOX(n); LOG("\n");
    if (*p != '\0') {        /// * not number
        t.compile = false;   ///> reset to interpreter mode
        return -1;           ///> skip the entire input buffer
    }
    // is a number
    if (t.compile) {         /// * add literal when in compile mode
        gPool.mem_op(DOLIT); ///> dovar (+parameter field)
        gPool.mem_du(n);     ///> data storage (32-bit integer now)
    }
    else t.push(n);          ///> or, add value onto data stack
    return 0;
}
///
/// outer interpreter
///
void outer(Thread &t, const char *cmd, void(*callback)(int, const char*)) {
    fin.clear();                             /// clear input stream error bit if any
    fin.str(cmd);                            /// feed user command into input stream
    fout_cb = callback;
    fout.str("");                            /// clean output buffer, ready for next
    while (fin >> strbuf) {
        const char *idiom = strbuf.c_str();
        LOG(idiom); LOG("=>");
        IU m = gPool.get_method(idiom, t.cls);    /// search for word in current context
        if (m > 0) {						 ///> if handle method found
            Word *w = WORD(m);
            LOG(w->nfa()); LOG(" 0x"); LOX(m);
            if (t.compile && !w->immd) {     /// * in compile mode?
                gPool.mem_iu(m);             /// * add found word to new colon word
            }
            else t.dispatch(m);              /// * execute method (either Java or Forth)
            continue;
        }
        m = gPool.get_class(idiom);			 ///> try as a class name (vocabulary)
        if (m > 0) {                         ///
            LOG("class 0x"); LOX(m); LOG("\n");
        	t.cls = m;					     /// * switch class (context)
        }
        else if (handle_number(t, idiom)) {	 ///> try as a number
        	fout << idiom << "? " << ENDL;   ///> display error prompt
        }
    }
    ss_dump(t);
}

void send_to_con(int len, const char *msg) { LOG(msg); }
void forth_interpreter(Thread &t) {
	cout << endl;
	string line;
	while (getline(cin, line)) {             /// fetch line from user console input
		outer(t, line.c_str(), send_to_con);
	}
}

int jvm_setup(const char *fname) {
	Method _obj[] = { {"<init>", [](Thread &t){ t.pop(); }, false} };
    setvbuf(stdout, NULL, _IONBF, 0);
    fout_cb = send_to_con;
    ///
    /// populate memory pool
    ///
    gPool.register_class("Ucode", uCode.vtsz, uCode.vt);
    gPool.register_class("java/lang/Object", sizeof(_obj)/sizeof(Method), _obj, "Ucode");
    gPool.register_class("ej32/Forth", uForth.vtsz, uForth.vt, "java/lang/Object");
    gPool.register_class("ej32/ESP32", uESP32.vtsz, uESP32.vt, "ej32/Forth");
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
void mem_stat(Thread &t) {
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
void mem_stat(Thread &t) {}
void jvm_run() {
    ///
    /// instantiate main thread (TODO: single thread for now)
    ///
    LOG("\nmain()");
    IU mx = gPool.get_method("main");
    
    gT0.dispatch(mx);
}
#endif // ARDUINO

