#ifndef NANOJVM_JVM_H
#define NANOJVM_JVM_H
#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <cstdlib>      // strtol
#include "ucode.h"

struct VM {
	Ucode  &ucode;						/// microcode
	Klass  &cls;						/// class pool
	Thread &t0;							/// thread pool
    List<IU, 256>   rs;					/// return stack

	istringstream   fin;    			/// forth_in
	ostringstream   fout;   			/// forth_out
	void (*fout_cb)(int, const char*);  /// forth output callback function
	string strbuf;          // input string buffer

	VM(Ucode &u, Klass &c, Thread &t) : ucode(u), cls(c), t0(t) {}

	int find(const char *name) {		/// function searcher
		int w = cls.find(name);			/// in class dictionary
		return (w < 0) ? ucode.find(name) : w; // or microcode vtable
	}
	void outer(const char *cmd, void(*callback)(int, const char*));
    ///
    /// inner interpreter methods
    ///
	void exec(IU c) { nest(c); inner(); unnest(); }
    void nest(IU c);
    void inner();
    void unnest();
    ///
    /// debug helpers
    ///
    void words();
    void ss_dump();
    void mem_dump(IU p0, DU sz);
};
#endif // NANOJVM_JVM_H

