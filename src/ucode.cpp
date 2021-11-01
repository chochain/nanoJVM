#include "ucode.h"
///
/// macros for reduce verbosity
///
#define TopS32      (*(S32*)&t.tos)
#define TopS64      (*(S64*)&t.tos)
#define TopF32      (*(F32*)&t.tos)
#define TopF64      (*(F64*)&t.tos)
#define TopU32      (*(U32*)&t.tos)
#define TopU64      (*(U64*)&t.tos)
#define PushI(n)    (t.push((S32)n))        /** tos updated */
#define PushL(n)    (t.push((S64)n))        /** tos updated */
#define PushF(n)    (0)
#define PushD(n)    (0)
#define PushA(n)    (t.push((P32)n))
#define PopI()      ((S32)t.pop())          /** tos updated */
#define PopL()      ((S64)t.pop())          /** tos updated */
#define PopF()      (0)
#define PopD()      (0)
#define PopA()      ((P32)t.pop())
#define PopR()      ((P32)t.pop())
#define PopI2T()    ((S32)t.pop())
#define PopL2T()    ((S64)t.pop())
#define PopF2T()    (0)
#define PopD2T()    (0)
#define PopA2T()    ((P32)t.pop())
#define PopR2T()    ((P32)t.pop())
#define LoadI(i)    PushI((S32)t.load((U32)i, (S32)0))
#define LoadL(i)    PushL((S64)t.load((U32)i, (S64)0))
#define LoadF(i)    (0)
#define LoadD(i)    (0)
#define LoadA(i)    PushI((P32)t.load((U32)i, (P32)0))
#define StorI(i)    (t.store((U32)i, PopI()))
#define StorL(i)    (t.store((U32)i, PopL()))
#define StorF(i)    (0)
#define StorD(i)    (0)
#define StorA(i)    (t.store((U32)i, PopA()))
///
/// array access macros (CC: TODO)
///
#define GetI_A(i)   PushI((S32)0)
#define GetL_A(i)   PushL((S64)0)
#define GetF_A(i)   PushF((F32)0)
#define GetD_A(i)   PushD((F64)0)
#define GetA_A(i)   PushA((P32)0)
#define GetB_A(i)   PushI((U8)0)
#define GetC_A(i)   PushI((U16)0)
#define GetS_A(i)   PushI((S16)0)
#define PutI_A(i)   (0)
#define PutL_A(i)   (0)
#define PutF_A(i)   (0)
#define PutD_A(i)   (0)
#define PutA_A(i)   (0)
#define PutB_A(i)   (0)
#define PutC_A(i)   (0)
#define PutS_A(i)   (0)
///
/// Class method, field access macros
///
#define OFF           (t.wide ? t.getBE32() : t.getBE16())
#define OFF8          (t.getBE8())
#define GetI_S(n)     PushI(t.gl[(n)])
#define PutI_S(n,v)   (t.gl[(n)]=(v))
#define GetI_F(o,n)   PushI(t.gl[(n)])
#define PutI_F(o,n,v) (t.gl[(n)]=(v))

#define UCODE(s, g) { s, [](Thread &t){ g; }, 0 }
///
/// micro-code (built-in methods)
///
auto _invoke2 = [](Thread &t){
    t.invoke(2);
};
static Method _java[] = {
    ///
    /// @definegroup Constant ops (CC:TODO)
    /// @{
    /*00*/  UCODE("nop",         {}),
    /*01*/  UCODE("aconst_null", {}),
    /*02*/  UCODE("iconst_M1",   {}),
    /*03*/  UCODE("iconst_0",  PushI(0)),
    /*04*/  UCODE("iconst_1",  PushI(1)),
    /*05*/  UCODE("iconst_2",  PushI(2)),
    /*06*/  UCODE("iconst_3",  PushI(3)),
    /*07*/  UCODE("iconst_4",  PushI(4)),
    /*08*/  UCODE("iconst_5",  PushI(5)),
    /*09*/  UCODE("lconst_0",  PushL(0)),
    /*0A*/  UCODE("lconst_1",  PushL(1)),
    /*0B*/  UCODE("fconst_0",  PushF(0)),
    /*0C*/  UCODE("fconst_1",  PushF(1)),
    /*0D*/  UCODE("fconst_2",  PushF(2)),
    /*0E*/  UCODE("dconst_0",  PushD(0)),
    /*0F*/  UCODE("dconst_1",  PushD(1)),
    /// @}
    /// @definegroup Load ops (CC: TODO)
    /// @{
    /*10*/  UCODE("bipush",   PushI(OFF8)),
    /*11*/  UCODE("sipush",   PushI(OFF)),
    /*12*/  UCODE("ldc",      U8  c = OFF8),    // load pool[U8] constant (cannot reach class file from here:TODO)
    /*13*/  UCODE("ldcw",     U16 c = OFF),     // load pool[U16] constant
    /*14*/  UCODE("ldc2_w",   {}),
    /*15*/  UCODE("iload",    LoadI(OFF8)),
    /*16*/  UCODE("lload",    LoadL(OFF8)),
    /*17*/  UCODE("fload",    LoadF(OFF8)),
    /*18*/  UCODE("dload",    LoadD(OFF8)),
    /*19*/  UCODE("aload",    LoadA(OFF8)),
    /*1A*/  UCODE("iload_0",  LoadI(0)),		// load from local (auto)
    /*1B*/  UCODE("iload_1",  LoadI(1)),
    /*1C*/  UCODE("iload_2",  LoadI(2)),
    /*1D*/  UCODE("iload_3",  LoadI(3)),
    /*1E*/  UCODE("lload_0",  LoadL(0)),
    /*1F*/  UCODE("lload_1",  LoadL(1)),
    /*20*/  UCODE("lload_2",  LoadL(2)),
    /*21*/  UCODE("lload_3",  LoadL(3)),
    /*22*/  UCODE("fload_0",  LoadF(0)),
    /*23*/  UCODE("fload_1",  LoadF(1)),
    /*24*/  UCODE("fload_2",  LoadF(2)),
    /*25*/  UCODE("fload_3",  LoadF(3)),
    /*26*/  UCODE("dload_0",  LoadD(0)),
    /*27*/  UCODE("dload_1",  LoadD(1)),
    /*28*/  UCODE("dload_2",  LoadD(2)),
    /*29*/  UCODE("dload_3",  LoadD(3)),
    /*2A*/  UCODE("aload_0",  LoadA(0)),
    /*2B*/  UCODE("aload_1",  LoadA(1)),	    /// load ref from local (auto)
    /*2C*/  UCODE("aload_2",  LoadA(2)),
    /*2D*/  UCODE("aload_3",  LoadA(3)),
    /*2E*/  UCODE("iaload",   GetI_A()),        /// n = PopI(); a = PopR(); GetI_A(a, n)
    /*2F*/  UCODE("faload",   GetF_A()),
    /*30*/  UCODE("laload",   GetL_A()),
    /*31*/  UCODE("daload",   GetD_A()),
    /*32*/  UCODE("aaload",   GetA_A()),
    /*33*/  UCODE("baload",   GetB_A()),
    /*34*/  UCODE("caload",   GetC_A()),
    /*35*/  UCODE("saload",   GetS_A()),
    /// @}
    /// @definegroup Store ops (CC: TODO)
    /// @{
    /*36*/  UCODE("istore",   StorI(OFF8)),
    /*37*/  UCODE("lstore",   {}),
    /*38*/  UCODE("fstore",   {}),
    /*39*/  UCODE("dstore",   {}),
    /*3A*/  UCODE("astore",   {}),
    /*3B*/  UCODE("istore_0", StorI(0)),	/// save to local (auto)
    /*3C*/  UCODE("istore_1", StorI(1)),
    /*3D*/  UCODE("istore_2", StorI(2)),
    /*3E*/  UCODE("istore_3", StorI(3)),
    /*3F*/  UCODE("lstore_0", StorL(0)),
    /*40*/  UCODE("lstore_1", StorL(1)),
    /*41*/  UCODE("lstore_2", StorL(2)),
    /*42*/  UCODE("lstore_3", StorL(3)),
    /*43*/  UCODE("fstore_0", StorF(0)),
    /*44*/  UCODE("fstore_1", StorF(1)),
    /*45*/  UCODE("fstore_2", StorF(2)),
    /*46*/  UCODE("fstore_3", StorF(3)),
    /*47*/  UCODE("dstore_0", StorF(0)),
    /*48*/  UCODE("dstore_1", StorF(1)),
    /*49*/  UCODE("dstore_2", StorF(2)),
    /*4A*/  UCODE("dstore_3", StorF(3)),
    /*4B*/  UCODE("astore_0", StorA(0)),	// store reference
    /*4C*/  UCODE("astore_1", StorA(1)),
    /*4D*/  UCODE("astore_2", StorA(2)),
    /*4E*/  UCODE("astore_3", StorA(3)),
    /*4F*/  UCODE("iastore",  PutI_A()),    // n = PopI(); v = PopI(); a = PopR(); PutI_A(a, n, v)
    /*50*/  UCODE("lastore",  PutL_A()),
    /*51*/  UCODE("fastore",  PutF_A()),
    /*52*/  UCODE("dastore",  PutD_A()),
    /*53*/  UCODE("aastore",  PutA_A()),
    /*54*/  UCODE("bastore",  PutB_A()),
    /*55*/  UCODE("castore",  PutC_A()),
    /*56*/  UCODE("sastore",  PutS_A()),
    /// @}
    /// @definegroup Stack ops
    /// @{
    /*57*/  UCODE("pop",      PopI()),
    /*58*/  UCODE("pop2",     PopI(); PopI()),
    /*59*/  UCODE("dup",      PushI(TopS32)),
    /*5A*/  UCODE("dup_x1",   S32 n = t.ss.pop(); S32 s = TopS32; PushI(n); PushI(s)),
    /*5B*/  UCODE("dup_x2",   {}),
    /*5C*/  UCODE("dup2",     S32 n = TopS32; PushI(n); PushI(t.ss[-2]); TopS32 = n),
    /*5D*/  UCODE("dup2_x1",  {}),
    /*5E*/  UCODE("dup2_x2",  {}),
    /*5F*/  UCODE("swap",     S64 n = t.ss.pop(); PushI(n)),
    /// @}
    /// @definegroup ALU Arithmetic ops
    /// @{
    /*60*/  UCODE("iadd", TopS32 += t.ss.pop()),
    /*61*/  UCODE("ladd", {}),
    /*62*/  UCODE("fadd", {}),
    /*63*/  UCODE("dadd", {}),
    /*64*/  UCODE("isub", TopS32 = t.ss.pop() - TopS32),
    /*65*/  UCODE("lsub", {}),
    /*66*/  UCODE("fsub", {}),
    /*67*/  UCODE("dsub", {}),
    /*68*/  UCODE("imul", TopS32 *= t.ss.pop()),
    /*69*/  UCODE("lmul", {}),
    /*6A*/  UCODE("fmul", {}),
    /*6B*/  UCODE("dmul", {}),
    /*6C*/  UCODE("idiv", TopS32 = t.ss.pop() / TopS32),
    /*6D*/  UCODE("ldiv", {}),
    /*6E*/  UCODE("fdiv", {}),
    /*6F*/  UCODE("ddiv", {}),
    /*70*/  UCODE("irem", TopS32 = t.ss.pop() % TopS32),
    /*71*/  UCODE("lrem", {}),
    /*72*/  UCODE("frem", {}),
    /*73*/  UCODE("drem", {}),
    /*74*/  UCODE("ineg", TopS32 = -TopS32),
    /*75*/  UCODE("lneg", {}),
    /*76*/  UCODE("fneg", TopF32 = -TopF32),
    /*77*/  UCODE("dneg", {}),
    /// @}
    /// @definegroup ALU Logical ops
    /// @{
    /*78*/  UCODE("ishl", TopS32 = t.ss.pop() << TopS32),
    /*79*/  UCODE("lshl", {}),
    /*7A*/  UCODE("ishr", TopS32 = t.ss.pop() >> TopS32),
    /*7B*/  UCODE("lshr", {}),
    /*7C*/  UCODE("iushr",TopU32 = (U32)t.ss.pop() >> TopS32),
    /*7D*/  UCODE("lushr",{}),
    /*7E*/  UCODE("iand", TopU32 = t.ss.pop() & TopU32),
    /*7F*/  UCODE("land", {}),
    /*80*/  UCODE("ior",  TopU32 = t.ss.pop() | TopU32),
    /*81*/  UCODE("lor",  {}),
    /*82*/  UCODE("ixor", TopU32 = t.ss.pop() ^ TopU32),
    /*83*/  UCODE("lxor", {}),
    /*84*/  UCODE("iinc", TopS32 += OFF),
    /// @}
    /// @definegroup ALU Conversion ops
    /// @{
    /*85*/  UCODE("i2l",  {}),
    /*86*/  UCODE("i2f",  {}),
    /*87*/  UCODE("i2d",  PushD(PopI())),
    /*88*/  UCODE("l2i",  PushI(PopL())),
    /*89*/  UCODE("l2f",  {}),
    /*8A*/  UCODE("l2d",  {}),
    /*8B*/  UCODE("f2i",  {}),
    /*8C*/  UCODE("f2l",  {}),
    /*8D*/  UCODE("f2d",  {}),
    /*8E*/  UCODE("d2i",  {}),
    /*8F*/  UCODE("d2l",  {}),
    /*90*/  UCODE("d2f",  {}),
    /*91*/  UCODE("i2b",  PushI((S8)PopI())),
    /*92*/  UCODE("i2c",  PushI((U16)PopI())),
    /*93*/  UCODE("i2s",  PushI((S16)PopI())),
    /// @}
    /// @definegroup ALU Arithmetic Compare ops
    /// @{
    /*94*/  UCODE("lcmp",
                  S64 n = PopL();
                  S64 d = PopL() - n;
                  PushI(d < 0 ? 1 : ((d > 0) ? -1 : 0))),
    /*95*/  UCODE("fcmpl", {}),
    /*96*/  UCODE("fcmpg", {}),
    /*97*/  UCODE("dcmpl", {}),
    /*98*/  UCODE("dcmpg", {}),
    /// @}
    /// @definegroup ALU Logical Compare ops
    /// @{
    /*99*/  UCODE("ifeq", t.cjmp(PopI() == 0)),
    /*9A*/  UCODE("ifne", t.cjmp(PopI() != 0)),
    /*9B*/  UCODE("iflt", t.cjmp(PopI() <  0)),
    /*9C*/  UCODE("ifge", t.cjmp(PopI() >= 0)),
    /*9D*/  UCODE("ifgt", t.cjmp(PopI() >  0)),
    /*9E*/  UCODE("ifle", t.cjmp(PopI() <= 0)),
    /*9F*/  UCODE("if_icmpeq", DU n = PopI(); t.cjmp(PopI() == n)),
    /*A0*/  UCODE("if_icmpne", DU n = PopI(); t.cjmp(PopI() != n)),
    /*A1*/  UCODE("if_icmplt", DU n = PopI(); t.cjmp(PopI() <  n)),
    /*A2*/  UCODE("if_icmpge", DU n = PopI(); t.cjmp(PopI() >= n)),
    /*A3*/  UCODE("if_icmpgt", DU n = PopI(); t.cjmp(PopI() >  n)),
    /*A4*/  UCODE("if_icmple", DU n = PopI(); t.cjmp(PopI() <= n)),
    /*A5*/  UCODE("if_acmpeq", {}),
    /*A6*/  UCODE("if_acmpne", {}),
    /// @}
    /// @definegroup Branching ops
    /// @{
    /*A7*/  UCODE("goto",      t.jmp()),
    /*A8*/  UCODE("jsr",       PushI((P32)(t.IP + sizeof(U16))); t.jmp()),
    /*A9*/  UCODE("ret",       t.IP = (U8*)(P32)OFF),
    /*AA*/  UCODE("tableswitch",  {}),
    /*AB*/  UCODE("lookupswitch", {}),
    /// @}
    /// @definegroup Return ops
    /// @{
    /*AC*/  UCODE("ireturn",   S32 n = TopS32; t.ret(); PushI(n)),
    /*AD*/  UCODE("lreturn",   S64 n = TopS64; t.ret(); PushL(n)),
    /*AE*/  UCODE("freturn",   F32 n = TopF32; t.ret(); PushF(n)),
    /*AF*/  UCODE("dreturn",   F64 n = TopF64; t.ret(); PushD(n)),
    /*B0*/  UCODE("areturn",   P32 n = TopU32; t.ret(); PushA(n)),
    /*B1*/  UCODE("return",    t.ret()),
    /// @}
    /// @definegroup Field Fetch ops
    /// @{
    /*B2*/  UCODE("getstatic", GetI_S(OFF)),                           /// fetch from class variable
    /*B3*/  UCODE("putstatic", PutI_S(OFF, PopI())),                   /// store into class variable
    /*B4*/  UCODE("getfield",  IU o = PopI(); GetI_F(o, OFF)),		   /// fetch from instance variable
    /*B5*/  UCODE("putfield",  S32 v = PopI(); IU o = PopI(); PutI_F(o, OFF, v)),/// store into instance variable
    /// @}
    /// @definegroup Method/Interface Invokation ops
    /// @{
    /*B6*/  UCODE("invokevirtual",   t.invoke(0)),
    /*B7*/  UCODE("invokespecial",   t.invoke(1)),
	/*B8*/  UCODE("invokestatic",    t.invoke(2)),
    /*B9*/  UCODE("invokeinterface", t.invoke(3)),
    /*BA*/  UCODE("invokedynamic",   t.invoke(4)),
    /// @}
    /// @definegroup New and Array ops
    /// @{
    /*BB*/  UCODE("new",          t.class_new()),
    /*BC*/  UCODE("newarray",     {}),
    /*BD*/  UCODE("anewarray",    {}),
    /*BE*/  UCODE("arraylength",  {}),
    /*BF*/  UCODE("athrow",       {}),
    /// @}
    /// @definegroup Misc ops
    /// @{
    /*C0*/  UCODE("checkcast",    {}),
    /*C1*/  UCODE("instanceof",   {}),
    /*C2*/  UCODE("monitorenter", {}),
    /*C3*/  UCODE("monitorexit",  {}),
    /*C4*/  UCODE("wide",         t.wide = true),
    /*C5*/  UCODE("multianewarray", {}),
    /*C6*/  UCODE("ifnull",       t.cjmp(PopA() == 0)),
    /*C7*/  UCODE("ifnonnull",    t.cjmp(PopA() != 0)),
    /*C8*/  UCODE("goto_w",       t.jmp()),
    /*C9*/  UCODE("jsr_w",        PushI((P32)(t.IP + sizeof(U32))); t.jmp()),
};
///
/// microcode ROM, use extern by main program
///
Ucode gUcode(sizeof(_java)/sizeof(Method), _java);
