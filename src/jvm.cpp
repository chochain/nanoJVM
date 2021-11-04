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
        Word *w = (Word*)&pmem[idx];
        if (w->len==len && strcmp(w->nfa(), s)==0) return idx;
        idx = w->lfa;
    } while (idx);
    return 0;
}
///
/// return cls_obj if cls_name is NULL
///
IU Pool::get_class(const char *cls_name) {
    return cls_name ? find(cls_name, cls_root) : cls_root;
}
///
/// return m_root if m_name is NULL
///
IU Pool::get_method(const char *m_name, IU cls_id, bool supr) {
    Word *cls = (Word*)&pmem[cls_id ? cls_id : cls_root];
    IU m_idx = 0;
    while (cls) {
        m_idx = find(m_name, *(IU*)cls->pfa(CLS_VT));
        if (m_idx || !supr) break;
        cls = cls->lfa ? (Word*)&pmem[cls->lfa] : 0;
    }
    return m_idx;
}
///
/// method constructor
///
IU Pool::add_method(Method &vt, IU &m_root) {
    IU mid = pmem.idx;              /// store current method idx
    add_iu(m_root);                 /// link to previous method
    add_u8(STRLEN(vt.name));        /// method name length
    add_u8((U8)vt.flag);            /// method access control
    add_str(vt.name);               /// enscribe method name
    add_pu((PU)vt.xt);              /// encode function pointer
    return m_root = mid;            /// adjust method root
};
IU Pool::add_method(const char *m_name, U32 m_idx, U8 flag, IU &m_root) {
    IU mid = pmem.idx;              /// store current method idx
    add_iu(m_root);                 /// link to previous method
    add_u8(STRLEN(m_name));         /// method name length
    add_u8(flag);                   /// method access control
    add_str(m_name);                /// enscribe method name
    add_du((DU)m_idx);              /// encode function pointer
    return m_root = mid;            /// adjust method root
};
IU Pool::add_class(const char *name, const char *supr, IU m_root, U16 cvsz, U16 ivsz) {
    /// encode class
    IU cid = pmem.idx;              /// preserve class link
    add_iu(cls_root);               /// class linked list
    add_u8(STRLEN(name));           /// class name string length
    add_u8(0);                      /// public
    add_str(name);                  /// class name string
    add_iu(get_class(supr));        /// super class
    add_iu(0);                      /// interface
    add_iu(m_root);                 /// vt
    add_iu(cvsz);                   /// cvsz
    add_iu(cvsz);                   /// ivsz
    return cls_root = cid;
}
///
/// class contructor
///
void Pool::register_class(const char *name, int sz, Method *vt, const char *supr) {
    /// encode vtable
    IU m_root = 0;
    for (int i=0; i<sz; i++) {
        add_method(vt[i], m_root);
    }
    add_class(name, supr, m_root, 0, 0);
    if (jvm_root==0) jvm_root = m_root;
}
///
/// word constructor
///
void Pool::colon(const char *name) {
    int mid = pmem.idx;
    add_iu(jvm_root);
    add_u8(STRLEN(name));
    add_u8(0);
    add_str(name);
    jvm_root = mid;
}
///
/// Java Virtual Machine implementation
///
extern   Ucode  gUcode;                 /// Java microcode ROM
extern   Ucode  gForth;                 /// Forth microcode ROM
Pool     gPool;                         /// memory management unit

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
        Word *cls = (Word*)&gPool.pmem[cid];
        fout << "\n" << cls->nfa() << " " << cid << "::";
        IU mid = *(IU*)cls->pfa(CLS_VT);
        int i = 0;
        do {
            Word *m = (Word*)&gPool.pmem[mid];
            if ((i++%10)==0) { fout << ENDL; fout << "\t"; yield(); }
            fout << m->nfa() << " " << mid << " ";
            mid = m->lfa;
        } while (mid);
        cid = cls->lfa;
    } while (cid);
    fout << setbase(t.base) << ENDL;
}
void ss_dump(Thread &t) {
    if (t.compile) return;
    fout << setbase(t.base) << " <";
    for (int i=0; i<t.ss.idx; i++) { fout << t.ss[i] << " "; }
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
        fout << setw(4) << i << ": ";
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
    printf("%d\n", n);
    if (*p != '\0') {        /// * not number
        t.compile = false;   ///> reset to interpreter mode
        return -1;           ///> skip the entire input buffer
    }
    // is a number
    if (t.compile) {         /// * add literal when in compile mode
        gPool.add_iu(DOLIT); ///> dovar (+parameter field)
        gPool.add_du(n);     ///> data storage (32-bit integer now)
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
    fout_cb = callback;                      /// setup callback function
    fout.str("");                            /// clean output buffer, ready for next
    while (fin >> strbuf) {
        const char *idiom = strbuf.c_str();
        printf("%s=>",idiom);
        IU w = gPool.get_method(idiom, t.cls_id);    /// search for word in current context
        if (w > 0) {						 ///> if handle method found
            Word *m = (Word*)&gPool.pmem[w];
            printf("%s 0x%x\n", m->nfa(), w);
            if (t.compile && !m->immd) {     /// * in compile mode?
                gPool.add_iu(w);             /// * add found word to new colon word
            }
            else t.dispatch(w);              /// * execute method (either Java or Forth)
            continue;
        }
        w = gPool.get_class(idiom);			 ///> try as a class name (vocabulary)
        if (w > 0) {                         ///
            printf("class 0x%x\n", w);
        	t.cls_id = w;					 /// * switch class (context)
        }
        else if (handle_number(t, idiom)) {	 ///> try as a number
        	fout << idiom << "? " << ENDL;   ///> display error prompt
        }
    }
    ss_dump(t);
}
///
/// main program
///
#include <iostream>           /// cin, cout
int main(int ac, char* av[]) {
    static auto send_to_con = [](int len, const char *rst) { cout << rst; };
    if (ac <= 1) {
        fprintf(stderr,"Usage:> $0 file_name.class\n");
        return -1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    ///
    /// populate memory pool
    ///
    gPool.register_class("java/lang/Object", gUcode.vtsz, gUcode.vt);
    gPool.register_class("nanojvm/Forth", gForth.vtsz, gForth.vt, "java/lang/Object");
    ///
    /// instantiate Java class loader
    ///
    Loader ld;
    FILE *f = fopen(av[1], "rb");
    if (!f) {
        fprintf(stderr," Failed to open file\n");
        return -1;
    }
    ld.init(f);
    U16 cidx = ld.load_class();
    if (!cidx) return -1;
    ///
    /// instantiate main thread (TODO: single thread for now)
    ///
    Thread t0(ld, &gPool.pmem[0], cidx);

    cout << unitbuf << "nanoJVM v1 staring..." << endl;
#if 0
    printf("\nmain()");
    fout_cb  = send_to_con;
    IU midx  = gPool.get_method("main", cidx);
    t0.dispatch(midx);
#else
    string line;
    while (getline(cin, line)) {             /// fetch line from user console input
        outer(t0, line.c_str(), send_to_con);
    }
#endif
    cout << "\n\nnanoJVM done." << endl;
    return 0;
}
