#include <iomanip>
#include "core.h"

#define CELL(a)   (*(DU*)&pmem[a])          /** fetch a cell from parameter memory       */
#define STR(a)    ((char*)&pmem[a])         /** fetch string pointer to parameter memory */
#define HERE      (pmem.idx)                /** current parameter memory index           */

int Klass::handle_number(Thread &t, const char *idiom) {
    char *p;
    int n = static_cast<int>(strtol(idiom, &p, t.base));
    printf("%d\n", n);
    if (*p != '\0') {                    /// * not number
        t.compile = false;               ///> reset to interpreter mode
        return -1;                       ///> skip the entire input buffer
    }
    // is a number
    if (t.compile) {                     /// * add literal when in compile mode
        //add_iu(DOLIT);                   ///> dovar (+parameter field)
        add_iu(0);                   ///> dovar (+parameter field)
        add_du(n);                       ///> data storage (32-bit integer now)
    }
    else t.ss.push(n);                   ///> or, add value onto data stack
    return 0;
}

void Klass::colon(const char *name) {
    char *nfa = STR(HERE);                  // current pmem pointer
    int sz = STRLEN(name);                  // string length, aligned
    pmem.push((U8*)name,  sz);              // setup raw name field
    Method m(nfa, NULL);
    m.def = 1;                              // specify a colon word
    m.pfa = HERE;                           // capture code field index
    dict.push(m);                           // deep copy Code struct into dictionary
};
