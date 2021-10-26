#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <string>       // string class
#include <stdlib.h>     // strtol
#include "ucode.h"
#include "jvm.h"
using namespace std;    // default to C++ standard template library
///
/// search for word following given linked list
///
IU Pool::find(const char *s, IU idx) {
	U8 len = STRLEN(s); // check length first, speed up matching
	do {
		Word *w = (Word*)&dict[idx];
        if (w->len==len && strcmp(w->nfa(), s)==0) return idx;
        idx = w->lfa;
    } while (idx);
    return 0;
}
///
/// return jvm_root if cls_name is NULL
///
IU Pool::get_class(const char *cls_name) {
    return cls_name ? find(cls_name, cls_root) : jvm_root;
}
///
/// return m_root if m_name is NULL
///
IU Pool::get_method(const char *m_name, const char *cls_name) {
    Word *cls   = (Word*)&dict[get_class(cls_name)];
    IU   m_root = *(IU*)cls->pfa();
    return m_name ? find(m_name, m_root) : m_root;
}
IU Pool::add_method(Method &vt, IU &m_root) {
	IU mid = dict.idx;				/// store current method idx
	U8 f   = vt.immd ? 0x2 : 0;
    add_iu(m_root);            		/// link to previous method
    add_u8(STRLEN(vt.name));		/// method name length
    add_u8(f);                      /// method access control
    add_str(vt.name);       		/// enscribe method name
    add_pu((PU)vt.xt);				/// encode function pointer
    return m_root = mid;	   		/// adjust method root
};
IU Pool::register_class(const char *name, int sz, Method *vt, const char *supr) {
    /// encode vtable
    IU m_root = supr ? get_method(NULL, supr) : 0;
    for (int i=0; i<sz; i++) {
    	add_method(vt[i], m_root);
    }
    /// encode class
    IU cid = dict.idx;				/// preserve class link
    add_iu(cls_root);
    add_u8(STRLEN(name));          	/// class name length
    add_u8(0);                     	/// class access control
    add_str(name);					/// enscribe class name
    add_iu(m_root);                	/// set root of method linked list
    
    if (jvm_root==0) jvm_root = m_root;
    return cls_root = cid;      	/// new class root
}
void Pool::colon(const char *name) {
	int mid = dict.idx;
	add_iu(jvm_root);
	add_u8(STRLEN(name));
	add_u8(0);
	add_str(name);
	jvm_root = mid;
}

extern   Ucode gUcode;				/// JVM microcodes
extern   Ucode gForth;				/// FVM microcodes
Pool     gPool;                     /// memory management unit
Thread   t0(&gPool.dict[0]);        /// one thread for now
List<DU, RS_SZ> rs;           		/// return stack

istringstream   fin;                /// forth_in
ostringstream   fout;               /// forth_out
string strbuf;                      /// input string buffer
void (*fout_cb)(int, const char*);  /// forth output callback function

#define ENDL    endl; fout_cb(fout.str().length(), fout.str().c_str()); fout.str("")
///
/// debug helper
///
void words() {
	fout << setbase(16);
    IU cid = gPool.cls_root;
    do {
    	Word *cls = (Word*)&gPool.dict[cid];
    	fout << "\n" << cls->nfa() << " " << cid << "::";
    	IU mid = *(IU*)cls->pfa();
    	int i = 0;
    	do {
        	Word *m = (Word*)&gPool.dict[mid];
            if ((i++%10)==0) { fout << ENDL; fout << "\t"; yield(); }
            fout << m->nfa() << " " << mid << " ";
            mid = m->lfa;
    	} while (mid);
    	cid = cls->lfa;
    } while (cid);
	fout << setbase(10) << ENDL;
}
void ss_dump() {
    if (t0.compile) return;
    fout << " <"; for (int i=0; i<t0.ss.idx; i++) { fout << t0.ss[i] << " "; }
    fout << t0.tos << "> ok" << ENDL;
    yield();
}
///
/// dump pmem at p0 offset for sz bytes
///
void mem_dump(IU p0, DU sz) {
    fout << setbase(16) << setfill('0') << ENDL;
    for (IU i=ALIGN16(p0); i<=ALIGN16(p0+sz); i+=16) {
        fout << setw(4) << i << ": ";
        for (int j=0; j<16; j++) {
            U8 c = gPool.dict[i+j];
            fout << setw(2) << (int)c << (j%4==3 ? "  " : " ");
        }
        for (int j=0; j<16; j++) {   // print and advance to next byte
            U8 c = gPool.dict[i+j] & 0x7f;
            fout << (char)((c==0x7f||c<0x20) ? '_' : c);
        }
        fout << ENDL;
        yield();
    }
    fout << setbase(10);
}
///
/// outer interpreter
///
void inner(IU w);
void CALL(IU w) {
	Word *m = (Word*)&gPool.dict[w];
	if (m->def) inner(w);
	else {
		fop xt = *(fop*)m->pfa();
		xt(t0);
	}
}
char *next_word()  {
    fin >> strbuf;
    return (char*)strbuf.c_str();
}
char *scan(char c) {
    getline(fin, strbuf, c);
    return (char*)strbuf.c_str();
}
void inner(IU w)    {
	rs.push(t0.IP - &gPool.dict[0]);
	rs.push(t0.WP = w);
    Word *m = (Word*)&gPool.dict[w];
    t0.IP = m->pfa();
    for (IU w1=*(IU*)t0.IP; t0.IP; t0.IP += sizeof(IU)) {
        CALL(w1);
    }
	t0.WP = (IU)rs.pop();
	t0.IP = &gPool.dict[0] + rs.pop();
}
int handle_number(const char *idiom) {
    char *p;
    int n = static_cast<int>(strtol(idiom, &p, t0.base));
    printf("%d\n", n);
    if (*p != '\0') {        /// * not number
        t0.compile = false;  ///> reset to interpreter mode
        return -1;           ///> skip the entire input buffer
    }
    // is a number
    if (t0.compile) {        /// * add literal when in compile mode
        gPool.add_iu(DOLIT); ///> dovar (+parameter field)
        gPool.add_du(n);     ///> data storage (32-bit integer now)
    }
    else t0.push(n);         ///> or, add value onto data stack
    return 0;
}
///
/// outer interpreter
///
void outer(const char *cmd, void(*callback)(int, const char*)) {
    fin.clear();                             /// clear input stream error bit if any
    fin.str(cmd);                            /// feed user command into input stream
    fout_cb = callback;                      /// setup callback function
    fout.str("");                            /// clean output buffer, ready for next
    while (fin >> strbuf) {
        const char *idiom = strbuf.c_str();
        printf("%s=>",idiom);
        int w = gPool.get_method(idiom);	 /// search forth words
        if (w > 0) {
            Word *m = (Word*)&gPool.dict[w];
            printf("%s 0x%x\n", m->nfa(), w);
            if (t0.compile && !m->immd) {    /// * in compile mode?
                gPool.add_iu(w);             /// * add found word to new colon word
            }
            else CALL(w);                    /// * execute word
        }
        else if (handle_number(idiom)) {
            fout << idiom << "? " << ENDL;   ///> display error prompt
        }
    }
    ss_dump();
}
///
/// main program
///
#include <iostream>         // cin, cout
int main(int ac, char* av[]) {
    static auto send_to_con = [](int len, const char *rst) { cout << rst; };

    cout << unitbuf << "nanoJVM v1" << endl;

    gPool.register_class("Ucode", gUcode.vtsz, gUcode.vt);
    gPool.register_class("nanojvm/Forth", gForth.vtsz, gForth.vt, "Ucode");

    string line;
    while (getline(cin, line)) {             /// fetch line from user console input
        outer(line.c_str(), send_to_con);
    }
    cout << "Done." << endl;
    return 0;
}
