#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <cstdlib>      // strtol
#include "ucode.h"
#include "jvm.h"        // VM namespace

#define IPOFF       (t.IP - t.M0)        /** calculate IP offset to memory base */
#define CELL(a)     (*(DU*)(t.M0 + a))   /** fetch a cell from parameter memory */
#define CODE(s, g)  { s, [](Thread &t){ g; }, false }
#define IMMD(s, g)  { s, [](Thread &t){ g; }, true  }

static Method _word[] = {
    ///
    /// @definegroup Forth Core
    /// @{
	/*CA*/  CODE("dovar", t.push(IPOFF); t.IP += sizeof(DU)),
    /*CB*/  CODE("dolit", t.push(IPOFF); t.IP += sizeof(DU)),
    /*CC*/  CODE("dostr",
                const char *s = (const char*)t.IP;
                t.push(IPOFF); t.IP += STRLEN(s)),
    /*CD*/  CODE("create",
                gPool.colon(next_word());
                gPool.add_iu(DOVAR)),
    /*CE*/  CODE("variable",
                gPool.colon(next_word());
                DU n = 0;
                gPool.add_iu(DOVAR);
                gPool.add_du(n)),
    /*CF*/  CODE("constant",
                gPool.colon(next_word());
                gPool.add_iu(DOLIT);
                gPool.add_du(t.pop())),
    /*D0*/  CODE(":",     gPool.colon(next_word()); t.compile=true),
    /*D1*/  IMMD(";",     t.compile = false),
    /*D2*/  CODE("@",     IU w = t.pop(); t.push(CELL(w))),          // w -- n
    /*D3*/  CODE("!",     IU w = t.pop(); CELL(w) = t.pop();),       // n w --
    /*D4*/  CODE(",",     DU n = t.pop(); gPool.add_du(n)),
    /*D5*/  CODE("allot", DU v = 0; for (IU n = t.pop(), i = 0; i < n; i++) gPool.add_du(v)), // n --
    /*D6*/  CODE("+!",    IU w = t.pop(); CELL(w) += t.pop()),       // n w --
    /*D7*/  CODE("here",  t.push(gPool.dict.idx)),
    /*D8*/  CODE("words", words()),
    /*D9*/  CODE(".s",    ss_dump()),
    /*DA*/  CODE("dump",  DU n = t.pop(); IU a = t.pop(); mem_dump(a, n)),
    /*DB*/  CODE("'",     IU w = gPool.get_method(next_word()); t.push(w)),
    /*DC*/  CODE("clock", t.push(millis())),
    /*DD*/  CODE("delay", delay(t.pop())),
    /*DE*/  CODE("bye",   exit(0))
    /// @}
};
///
/// Forth built-in word in ROM, use extern by main program
///
Ucode gForth(sizeof(_word)/sizeof(Method), _word);


