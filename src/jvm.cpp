#include "jvm.h"
using namespace std;
///
/// debug helper
///
void VM::words() {
    fout << setbase(16);
    for (int i=0; i<cls.dict.idx; i++) {
        if ((i%10)==0) { fout << ENDL; }
        fout << cls.dict[i].name << " ";
    }
    fout << setbase(10);
}
void VM::ss_dump() {
    if (t0.compile) return;
    fout << " <"; for (int i=0; i<t0.ss.idx; i++) { fout << t0.ss[i] << " "; }
    fout << t0.tos << "> ok" << ENDL;
}
///
/// dump pmem at p0 offset for sz bytes
///
void VM::mem_dump(IU p0, DU sz) {
    fout << setbase(16) << setfill('0') << ENDL;
    for (IU i=ALIGN16(p0); i<=ALIGN16(p0+sz); i+=16) {
        fout << setw(4) << i << ": ";
        for (int j=0; j<16; j++) {
            U8 c = pmem[i+j];
            fout << setw(2) << (int)c << (j%4==3 ? "  " : " ");
        }
        for (int j=0; j<16; j++) {   // print and advance to next byte
            U8 c = pmem[i+j] & 0x7f;
            fout << (char)((c==0x7f||c<0x20) ? '_' : c);
        }
        fout << ENDL;
    }
    fout << setbase(10);
}
///
/// outer interpreter
///
void VM::nest(IU c) {
	rs.push(t0.IP - (U8*)&t0);
	rs.push(t0.WP=c);
	t0.IP=&pmem[cls.dict[c].pfa];
}
void VM::inner()    {
	for (IU c=*t0.IP; t0.IP; t0.IP+=sizeof(IU)) {
		exec(c);
	}
}
void VM::unnest() {
	t0.WP = (IU)rs.pop();
	t0.IP = (U8*)&t0 + rs.pop();
}
///
/// outer interpreter
///
void VM::outer(const char *cmd, void(*callback)(int, const char*)) {
    fin.clear();                             /// clear input stream error bit if any
    fin.str(cmd);                            /// feed user command into input stream
    fout_cb = callback;                      /// setup callback function
    fout.str("");                            /// clean output buffer, ready for next
    while (fin >> strbuf) {
        const char *idiom = strbuf.c_str();
        //printf("%s=>",idiom);
        int w = find(idiom);                 /// * search through dictionary
        if (w >= 0) {                        /// * word found?
        	if (w & 0xf000) {
        		w &= ~0xf000;
        	    printf("%s %d\n", cls.dict[w].name, w);
        	    if (t0.compile && !cls.dict[w].immd) {    /// * in compile mode?
        	        cls.add_iu(w);                        /// * add found word to new colon word
        	    }
        	    else exec(w);				/// * execute word
        	}
        	else ucode.call(t0, w);
            continue;
        }
        // handle as a number
        if (cls.handle_number(t0, idiom)) {
			fout << idiom << "? " << ENDL;   ///> display error prompt
        }
    }
    ss_dump();
}
///
/// main program
///
#include <iostream>     // cin, cout
extern Ucode             ucode;              /// microcode ROM
static List<U8, HEAP_SZ> heap;				 /// heap space

int main(int ac, char* av[]) {
    static auto send_to_con = [](int len, const char *rst) { cout << rst; };

    cout << unitbuf << "nanoJVM v1" << endl;
    Klass  cls("Object", heap);
    Thread th;
    VM     vm(ucode, cls, th, heap);
    string line;
    while (getline(cin, line)) {             /// fetch line from user console input
        vm.outer(line.c_str(), send_to_con);
    }
    cout << "Done." << endl;
    return 0;
}
