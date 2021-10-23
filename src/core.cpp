#include <iomanip>
#include "core.h"

#include <sstream>      // iostream, stringstream
#include <iomanip>      // setbase
#include <cstdlib>      // strtol
#define CODE(s, g)  { s, [](Thread &t){ g; }, false }
#define IMMD(s, g)  { s, [](Thread &t){ g; }, true  }
#if 0
static Method word[] = {
    ///
    /// @definegroup Forth Core
    /// @{
    /*00*/  CODE("dovar",   PushI(IPOFF);    t.IP += sizeof(DU)),
    /*01*/  CODE("dolit",   PushI(*(DU*)IP); t.IP += sizeof(DU)),
    /*02*/  CODE("dostr",
                const char *s = (const char*)IP;
                PushI(IPOFF); IP += STRLEN(s)),
    /*03*/  CODE("create",
                colon(next_word());
                add_iu(find("dovar"))),
    /*04*/  CODE("variable",
                colon(next_word());
                DU n = 0;
                add_iu(find("dovar"));
                add_du(n)),
    /*05*/  CODE("constant",
                colon(next_word());
                add_iu(find("dolit"));
                add_du(POP())),
    /*06*/  CODE(":", t.colon(next_word()); t.compile=true),
    /*07*/  IMMD(";", t.compile = false),
    /*08*/  CODE("bye", exit(0))
    /// @}
};
#endif

