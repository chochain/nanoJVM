#include "ucode.h"
#include "mmu.h"
#include "jvm.h"        // VM namespace

#define CELL(a)     (*(DU*)(t.M0 + a))   /** fetch a cell from parameter memory */
#define CODE(s, g)  { s, [](Thread &t){ g; }, 0 }
#define IMMD(s, g)  { s, [](Thread &t){ g; }, FLAG_IMMD }
#define POP         t.pop()
#define PUSH(v)     t.push(v)

static Method _word[] = {
    ///
    /// @definegroup Forth Core
    /// @{
    /*CB*/  CODE("dovar", PUSH(t.IP); t.IP += sizeof(DU)),
    /*CC*/  CODE("dolit", PUSH(CELL(t.IP)); t.IP += sizeof(DU)),
    /*CD*/  CODE("dostr",
                const char *s = (const char*)(t.M0 + t.IP);
                PUSH(t.IP); t.IP += STRLEN(s)),
    /*CE*/  CODE("unnest", t.IP = 0),
    /*CF*/  CODE("create",
                gPool.colon(t, next_word());
                gPool.add_op(DOVAR)),
    /*D0*/  CODE("variable",
                gPool.colon(t, next_word());
                DU n = 0;
                gPool.add_op(DOVAR);
                gPool.add_du(n)),
    /*D1*/  CODE("constant",
                gPool.colon(t, next_word());
                gPool.add_op(DOLIT);
                gPool.add_du(POP)),
    /*D2*/  CODE(":",     gPool.colon(t, next_word()); t.compile=true),
    /*D3*/  IMMD(";",     gPool.add_op(UNNEST); t.compile = false),
    /*D4*/  CODE("@",     IU w = POP; PUSH(CELL(w))),          // w -- n
    /*D5*/  CODE("!",     IU w = POP; CELL(w) = POP;),       // n w --
    /*D6*/  CODE(",",     DU n = POP; gPool.add_du(n)),
    /*D7*/  CODE("allot", DU v = 0; for (IU n = POP, i = 0; i < n; i++) gPool.add_du(v)), // n --
    /*D8*/  CODE("+!",    IU w = POP; CELL(w) += POP),       // n w --
    /*D9*/  CODE("decimal", t.base = 10),
    /*DA*/  CODE("hex",     t.base = 16),
    /*DB*/  CODE("here",  PUSH(gPool.pmem.idx)),
    /*DC*/  CODE("words", words(t)),
    /*DE*/  CODE("ss",    ss_dump(t)),
    /*DF*/  CODE("dump",  DU n = POP; IU a = POP; mem_dump(t, a, n)),
    /*E0*/  CODE("tick",  IU w = gPool.get_method(next_word(), t.cls); PUSH(w)),
    /*E1*/  CODE("clock", PUSH(millis())),
    /*E2*/  CODE("delay", delay(POP)),
    /*E3*/  CODE("interpreter", forth_interpreter(t)),
    /*E4*/  CODE("bye",   exit(0))
    /// @}
};
///
/// Forth built-in word in ROM, use extern by main program
///
Ucode uForth(sizeof(_word)/sizeof(Method), _word);


