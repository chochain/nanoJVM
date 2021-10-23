#include <iomanip>
#include "jvm.h"

#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <cstdlib>      // strtol

extern const char *next_word();

#define IPOFF       (t.IP - t.M0)
#define CODE(s, g)  { s, [](Thread &t){ g; }, false }
#define IMMD(s, g)  { s, [](Thread &t){ g; }, true  }

enum { DOVAR = 0xCA, DOLIT, DOSTR }; 	/// Forth opcodes

static Method word[] = {
    ///
    /// @definegroup Forth Core
    /// @{
	/*CA*/  CODE("dovar",   t.push(IPOFF); t.IP += sizeof(DU)),
    /*CB*/  CODE("dolit",   t.push(IPOFF); t.IP += sizeof(DU)),
    /*CC*/  CODE("dostr",
                const char *s = (const char*)t.IP;
                t.push(IPOFF); t.IP += STRLEN(s)),
    /*CD*/  CODE("create",
                gPool.colon(next_word());
                gPool.add_iu(DOVAR)),
    /*CF*/  CODE("variable",
                gPool.colon(next_word());
                DU n = 0;
                gPool.add_iu(DOVAR);
                gPool.add_du(n)),
    /*D0*/  CODE("constant",
                gPool.colon(next_word());
                gPool.add_iu(DOLIT);
                gPool.add_du(t.pop())),
    /*D1*/  CODE(":", gPool.colon(next_word()); t.compile=true),
    /*D2*/  IMMD(";", t.compile = false),
    /*D3*/  CODE("bye", exit(0))
    /// @}
};

