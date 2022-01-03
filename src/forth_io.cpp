#include <sstream>      // stringstream
#include <iostream>     // cin, cout
#include <iomanip>      // setbase
#include <string>       // string class (for TIB)
#include "ucode.h"      // DOLIT
#include "mmu.h"		// memory pool manager
#include "forth_io.h"	// Forth IO interface

using namespace std;    // default to C++ standard template library
void send_to_con(int len, const char *msg) { LOG(msg); }
///
/// JVM streaming IO
///
istringstream   fin;                    /// forth_in
ostringstream   fout;                   /// forth_out
string          tib;                    /// terminal input buffer
void (*fout_cb)(int, const char*) = send_to_con;      /// forth output callback function

#define ENDL    endl; fout_cb(fout.str().length(), fout.str().c_str()); fout.str("")
///
/// debug helper
///
void words(Thread &t) {
    fout << setbase(16);
    IU cid = gPool.cls_root;
    do {
        Word *cls = WORD(cid);
        IU    sid = *(IU*)cls->pfa(PFA_CLS_SUPR);
        IU    mid = *(IU*)cls->pfa(PFA_CLS_VT);
        fout << "\n" << cls->nfa() << " " << cid << " << "
             << (sid == DATA_NA ? "-" : WORD(sid)->nfa());
        int i = 0;
        do {
            Word *w = WORD(mid);
            if ((i++%10)==0) { fout << ENDL; fout << "\t"; yield(); }
            fout << w->nfa() << " " << mid << " ";
            mid = w->lfa;
        } while (mid != DATA_NA);
        cid = cls->lfa;
    } while (cid != DATA_NA && cid != gPool.jvm_root);
    fout << setbase(t.base) << ENDL;
}
void ss_dump(Thread &t) {
    if (t.compile) return;
    fout << setbase(t.base) << " <";
    for (int i=0; i<t.ss.idx; i++) {
    	fout << t.ss[i] << ((i+1)==t.SP ? "|" : " ");
    }
    fout << t.TOS << "> ok" << ENDL;
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
    fin >> tib;
    return (char*)tib.c_str();
}
char *scan(char c) {
    getline(fin, tib, c);
    return (char*)tib.c_str();
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
void outer(Thread &t, const char *cmd) {
    fin.clear();                             /// clear input stream error bit if any
    fin.str(cmd);                            /// feed user command into input stream
    fout.str("");                            /// clean output buffer, ready for next
    while (fin >> tib) {
        const char *idiom = tib.c_str();
        LOG(idiom); LOG("=>");
        IU m = gPool.get_method(idiom, 0, t.cls);    /// search for word in current context
        if (m != DATA_NA) {					 ///> if handle method found
            Word *w = WORD(m);
            LOG(w->nfa()); LOG(" 0x"); LOX(m);
            if (t.compile && !w->immd) {     /// * in compile mode?
                gPool.mem_iu(m);             /// * add found word to new colon word
            }
            else t.dispatch(m);              /// * execute method (either Java or Forth)
            continue;
        }
        m = gPool.get_class(idiom);			 ///> try as a class name (vocabulary)
        if (m != DATA_NA) {                  ///
            LOG("class 0x"); LOX(m); LOG("\n");
        	t.cls = m;					     /// * switch class (context)
        }
        else if (handle_number(t, idiom)) {	 ///> try as a number
        	fout << idiom << "? " << ENDL;   ///> display error prompt
        }
    }
    ss_dump(t);
}

void forth_interpreter(Thread &t) {
	cout << endl;
	string line;
	while (getline(cin, line)) {             /// fetch line from user console input
		outer(t, line.c_str());
	}
}

