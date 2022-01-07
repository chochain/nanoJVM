#ifndef NANOJVM_THREAD_H
#define NANOJVM_THREAD_H
#include "core.h"           /// List
#include "loader.h"         /// loader and common types
///
/// Thread class
///
struct Thread {
    List<DU, SS_SZ>  ss;    /// data stack
    Loader &J;              /// Java class loader
    U8     *M0    = NULL;   /// cached base address of memory pool

    bool  compile = false;  /// compile flag
    bool  wide    = false;  /// wide flag
    DU    base    = 10;     /// radix
    IU    cls     = 0;      /// current class, =~ Forth context
    DU    TOS     = -1;     /// top of stack
    IU    IP      = 0;      /// instruction pointer (program counter)
    U16   SP      = 0;      /// local stack frame index

    Thread(Loader &ldr) : J(ldr) {}

    void init(U8 *mem, IU cx) { M0 = mem; cls = cx; }
    ///
    /// VM Execution Unit
    ///
    void na();                           /// not supported
    void dispatch(IU mx, U16 nparm=0);   /// instruction dispatcher dispatch
    ///
    /// Java core opcodes
    ///
    void java_new();                     /// instantiate Java object
    void java_call(IU j, U16 nparm=0);   /// execute Java method
    void invoke(U16 itype);              /// invoke type: 0:virtual, 1:special, 2:static, 3:interface, 4:dynamic
    ///
    /// class and instance variable access
    ///
    DU   *cls_var();
    DU   *inst_var(IU ox);
    ///
    /// Java array opcodes
    ///
    void java_newa(IU n);                /// instantiate Java array
    void java_anewa(IU n);               /// create multi-dimension array
    IU   alen(IU ax);                    /// return array length
    void astore(IU ax, IU idx, DU v);    /// store v into array[idx]
    DU   *aload(IU ax, IU idx);          /// fetch v from array[idx]
    ///
    /// Java class file byte fetcher
    ///
    U8   fetch()        { return J.getU8(IP++); }
    U16  fetch2()       { U16 n = J.getU16(IP);  IP+=2; LOG(" #"); LOX(n); return n; }
    U16  fetch4()       { U32 n = J.getU32(IP);  IP+=4; LOG(" #"); LOX(n); return n; }
    ///
    /// branching ops
    ///
    void ret()          { IP = 0; }		  /// exit java_call/forth_word loop
    void jmp()          { IP += J.getU16(IP) - 1; }
    void cjmp(bool f)   { IP += f ? J.getU16(IP) - 1 : sizeof(U16); }
    ///
    /// stack ops
    ///
    void push(DU v)     { ss.push(TOS); TOS = v; }
    DU   pop()          { DU n = TOS; TOS = ss.pop(); return n; }
    ///
    /// local variable access
    ///
#if RANGE_CHECK
    void iinc() {
    	U8 i = fetch();
    	U8 v = fetch();
        if ((SP+i) > ss.idx) throw "ERR: iinc > ss.idx";
        ((SP+i)==ss.idx) ? TOS += v : ss[SP + i] += v;
    }
    template<typename T>
    T    load(U16 i, T n)  {
        if ((SP+i) > ss.idx) throw "ERR: load > ss.idx";
        return ((SP+i)==ss.idx) ? TOS : *(T*)&ss[SP + i];
    }
    template<typename T>
    void store(U16 i, T n) {
        if ((SP+i) > ss.idx) throw "ERR: store > ss.idx";
        ((SP+i)==ss.idx) ? TOS = n : *(T*)&ss[SP + i] = n;
    }
#else
    void iinc(U8 i, S8 v)  { ((SP+i)==ss.idx) ? TOS += v : ss[SP + i] += v; }
    template<typename T>
    T    load(U16 i, T n)  { return ((SP+i)==ss.idx) ? TOS : *(T*)&ss[SP + i]); }
    template<typename T>
    void store(U16 i, T n) { ((SP+i)==ss.idx) ? TOS = n : *(T*)&ss[SP + i] = n; }
#endif // RANGE_CHECK
};
#endif // NANOJVM_THREAD_H
