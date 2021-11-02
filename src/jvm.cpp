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
IU Pool::get_method(const char *m_name, const char *cls_name, bool supr) {
    Word *cls = (Word*)&pmem[get_class(cls_name)];
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
extern   Loader gLoader;                /// Java class loader
Pool     gPool;                         /// memory management unit
Thread   t0(gLoader, &gPool.pmem[0]);   /// one thread for now
List<DU, RS_SZ> rs;                     /// return stack

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
    fout << setbase(10);
}
///
/// outer interpreter
///
void inner(IU w);
void CALL(IU w) {
    Word *m = (Word*)&gPool.pmem[w];
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
    rs.push(t0.IP);
    rs.push(t0.WP = w);
    Word *m = (Word*)&gPool.pmem[w];
    t0.IP = (IU)(m->pfa() - t0.M0);
    for (IU w1=*(IU*)(t0.M0 + t0.IP); t0.IP; t0.IP += sizeof(IU)) {
        CALL(w1);
    }
    t0.WP = (IU)rs.pop();
    t0.IP = rs.pop();
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
        IU w = gPool.get_method(idiom, "nanojvm/Forth");    /// search forth words
        if (w > 0) {
            Word *m = (Word*)&gPool.pmem[w];
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
/// Java class file constant pool access macros
///
#define jU16(a)      J.getU16(a)
#define jOff(i)      J.poolOffset(i - 1)
#define jStrRef(i,s) J.getStr(i, s, true)
#define jStr(i,s)    J.getStr(i, s, false)
///
/// Thread class implementation
///
void Thread::java_new()  {
    IU idx = fetch2();              /// class index
    /// TODO: allocate space for the object instance
    char buf[128];
    printf(" %s", jStrRef(idx, buf));
    push(idx);                      /// save object on stack
}
void Thread::java_inner(IU addr) {
    U16 nloc = jU16(addr - 6);      /// TODO: allocate local stack frame
    rs.push(t0.IP);
    t0.IP = addr;                   /// pointer to class file
    while (t0.IP) {
        ss_dump();
        U8 op = fetch();            /// fetch opcode
        printf("%04x:%02x %s", t0.IP-1, op, gUcode.vt[op].name);
        gUcode.exec(t0, op);        /// execute JVM opcode
    }
    t0.IP = rs.pop();
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
        if (w->java) java_inner(addr);              /// call Java inner interpreter
        else         CALL(m);                       /// call Native method
    }
    else printf(" **NA**");
}
///
/// main program
///
#include <iostream>                 /// cin, cout

int main(int ac, char* av[]) {
    static auto send_to_con = [](int len, const char *rst) { cout << rst; };
    if (ac <= 1) {
        fprintf(stderr,"Usage:> $0 file_name.class\n");
        return -1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);

    gPool.register_class("Ucode", gUcode.vtsz, gUcode.vt);
    gPool.register_class("nanojvm/Forth", gForth.vtsz, gForth.vt, "Ucode");
    
    FILE *f = fopen(av[1], "rb");
    if (!f) {
        fprintf(stderr," Failed to open file\n");
        return -1;
    }
    gLoader.init(f);
    if (gLoader.load_class()) return -1;

    cout << unitbuf << "nanoJVM v1 staring...\n\nmain()";
#if 1
    fout_cb  = send_to_con;
    IU midx  = gPool.get_method("main");
    Word *w  = (Word*)&gPool.pmem[midx];
    IU  addr = *(IU*)w->pfa();
    t0.java_inner(addr);
#else
    string line;
    while (getline(cin, line)) {             /// fetch line from user console input
        outer(line.c_str(), send_to_con);
    }
#endif
    cout << "\n\nnanoJVM done." << endl;
    return 0;
}
