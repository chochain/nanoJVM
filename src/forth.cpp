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
    CODE("dovar", PUSH(t.IP); t.IP += sizeof(DU)),
    CODE("dolit", PUSH(CELL(t.IP)); t.IP += sizeof(DU)),
    CODE("dostr",
         const char *s = (const char*)(t.M0 + t.IP);
         PUSH(t.IP); t.IP += STRLEN(s)),
    CODE("unnest", t.IP = 0),
    CODE("create",
         gPool.colon(t, next_word());
         gPool.add_op(DOVAR)),
    CODE("variable",
         gPool.colon(t, next_word());
         DU n = 0;
         gPool.add_op(DOVAR);
         gPool.add_du(n)),
    CODE("constant",
         gPool.colon(t, next_word());
         gPool.add_op(DOLIT);
         gPool.add_du(POP)),
    CODE(":",     gPool.colon(t, next_word()); t.compile=true),
    IMMD(";",     gPool.add_op(UNNEST); t.compile = false),
    CODE("@",     IU w = POP; PUSH(CELL(w))),          // w -- n
    CODE("!",     IU w = POP; CELL(w) = POP;),       // n w --
    CODE(",",     DU n = POP; gPool.add_du(n)),
    CODE("allot", DU v = 0; for (IU n = POP, i = 0; i < n; i++) gPool.add_du(v)), // n --
    CODE("+!",    IU w = POP; CELL(w) += POP),       // n w --
    CODE("decimal", t.base = 10),
    CODE("hex",     t.base = 16),
    CODE("here",  PUSH(gPool.pmem.idx)),
    CODE("words", words(t)),
    CODE("ss",    ss_dump(t)),
    CODE("dump",  DU n = POP; IU a = POP; mem_dump(t, a, n)),
    CODE("tick",  IU w = gPool.get_method(next_word(), t.cls); PUSH(w)),
    CODE("clock", PUSH(millis())),
    CODE("delay", delay(POP)),
    CODE("interpreter", forth_interpreter(t)),
    CODE("bye",   exit(0))
    /// @}
};
///
/// Forth built-in word in ROM, use extern by main program
///
Ucode uForth(sizeof(_word)/sizeof(Method), _word);


