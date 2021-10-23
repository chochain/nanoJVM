#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <string>       // string class
#include <stdlib.h>     // strtol
#include "ucode.h"		// gUcode microcode unit
#include "jvm.h"
using namespace std;    // default to C++ standard template library

static Pool     gPool;  // memory management unit
static Thread   t0;     // one thread for now
List<DU, RS_SZ> rs;     // return stack

istringstream   fin;    // forth_in
ostringstream   fout;   // forth_out
string strbuf;          // input string buffer
void (*fout_cb)(int, const char*);  // forth output callback function

#define ENDL    endl; fout_cb(fout.str().length(), fout.str().c_str()); fout.str("")
///
/// debug helper
///
void words() {
    fout << setbase(16);
    for (int i=0; i<gPool.dict.idx; i++) {
        if ((i%10)==0) { fout << ENDL; }
        fout << gPool.dict[i].name << " ";
    }
    fout << setbase(10);
}
void ss_dump() {
    if (t0.compile) return;
    fout << " <"; for (int i=0; i<t0.ss.idx; i++) { fout << t0.ss[i] << " "; }
    fout << t0.tos << "> ok" << ENDL;
}
///
/// dump pmem at p0 offset for sz bytes
///
void mem_dump(IU p0, DU sz) {
    fout << setbase(16) << setfill('0') << ENDL;
    for (IU i=ALIGN16(p0); i<=ALIGN16(p0+sz); i+=16) {
        fout << setw(4) << i << ": ";
        for (int j=0; j<16; j++) {
            U8 c = gPool.pmem[i+j];
            fout << setw(2) << (int)c << (j%4==3 ? "  " : " ");
        }
        for (int j=0; j<16; j++) {   // print and advance to next byte
            U8 c = gPool.pmem[i+j] & 0x7f;
            fout << (char)((c==0x7f||c<0x20) ? '_' : c);
        }
        fout << ENDL;
    }
    fout << setbase(10);
}
///
/// outer interpreter
///
#define CALL(c) \
	if (gPool.dict[c].def) inner(c); \
	else gUcode.call(t0, c)

void inner(IU c)    {
	rs.push(t0.IP - (U8*)&t0);
	rs.push(t0.WP=c);
	t0.IP = &gPool.pmem[gPool.dict[c].pfa];
	for (IU c1=*t0.IP; t0.IP; t0.IP += sizeof(IU)) {
		CALL(c1);
	}
	t0.WP = (IU)rs.pop();
	t0.IP = (U8*)&t0 + rs.pop();
}
int handle_number(const char *idiom) {
    char *p;
    int n = static_cast<int>(strtol(idiom, &p, t0.base));
    printf("%d\n", n);
    if (*p != '\0') {                   /// * not number
        t0.compile = false;             ///> reset to interpreter mode
        return -1;                      ///> skip the entire input buffer
    }
    // is a number
    if (t0.compile) {                   /// * add literal when in compile mode
        //add_iu(DOLIT);                ///> dovar (+parameter field)
        gPool.add_iu(0);                ///> dovar (+parameter field)
        gPool.add_du(n);                ///> data storage (32-bit integer now)
    }
    else t0.ss.push(n);                 ///> or, add value onto data stack
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
        int w = gPool.get_method(idiom);
        if (w < 0 && handle_number(idiom)) {
    		fout << idiom << "? " << ENDL;   ///> display error prompt
        }
        else {
        	Method *m = &gPool.dict[w];
            printf("%s %d\n", m->name, w);
            if (t0.compile && !m->immd) { 	 /// * in compile mode?
            	gPool.add_iu(w);           	 /// * add found word to new colon word
            }
            else CALL(w);		 			 /// * execute word
        }
    }
    ss_dump();
}
///
/// main program
///
#include <iostream>     	// cin, cout
int main(int ac, char* av[]) {
    static auto send_to_con = [](int len, const char *rst) { cout << rst; };

    cout << unitbuf << "nanoJVM v1" << endl;

    gPool.register_class("Object", gUcode.vtsz, gUcode.vt);
    string line;
    while (getline(cin, line)) {             /// fetch line from user console input
        outer(line.c_str(), send_to_con);
    }
    cout << "Done." << endl;
    return 0;
}
