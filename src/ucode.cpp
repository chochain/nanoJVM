#include "jvm.h"
///
/// macros for reduce verbosity
///
#define TopS32      (*(S32*)&t->top)
#define TopS64      (*(S64*)&t->top)
#define TopF32      (*(F32*)&t->top)
#define TopF64      (*(F64*)&t->top)
#define TopU32      (*(U32*)&t->top)
#define TopU64      (*(U64*)&t->top)
#define PushI(n)    t->push((S32)n)
#define PushL(n)    t->push((S64)n)
#define PushF(n)    (0)
#define PushD(n)    (0)
#define PushA(n)    t->push((P32)n)
#define PopI()      ((S32)t->pop())
#define PopL()      ((S64)t->pop())
#define PopF()      (0)
#define PopD()      (0)
#define PopA()      ((P32)t->pop())
#define PopR()      ((P32)t->pop())
#define LoadI(i)    (S32)t->load((U32)i, (S32)i)
#define LoadL(i)    (S64)t->load((U32)i, (S64)i)
#define LoadF(i)    (0)
#define LoadD(i)    (0)
#define LoadA(i)    (P32)t->load((U32)i, (P32)i)
#define StorI(i,n)  (t->store((U32)i, (S32)n))
#define StorL(i,n)  (t->store((U32)i, (S64)n))
#define StorF(i,n)  (0)
#define StorD(i,n)  (0)
#define StorR(i,n)  (t->store((U32)i, (P32)n))
///
/// array access macros (CC: TODO)
///
#define GetI_A(i,n)   ((S32)0)
#define GetL_A(i,n)   ((S64)0)
#define GetF_A(i,n)   ((F32)0)
#define GetD_A(i,n)   ((F64)0)
#define GetA_A(i,n)   ((P32)0)
#define GetB_A(i,n)   ((U8)0)
#define GetC_A(i,n)   ((U16)0)
#define GetS_A(i,n)   ((S16)0)
#define SetI_A(i,n,v) (0)
#define SetL_A(i,n,v) (0)
#define SetF_A(i,n,v) (0)
#define SetD_A(i,n,v) (0)
#define SetA_A(i,n,v) (0)
#define SetB_A(i,n,v) (0)
#define SetC_A(i,n,v) (0)
#define SetS_A(i,n,v) (0)
///
/// Class method, field access macros
///
#define GetI_S(n)   (0)
#define SetI_S(n,v) (0)
#define GetI_F(n)   (0)
#define SetI_F(n,v) (0)
#define Jump(a)     (0)
#define Dup(a, b)   (0)
#define Ret()       (_Return())
#define Nest(n)     (_Invoke(n))
#define OFF         (t->wide)

U8  GetB_C(U32 i)   { return 0; }
U16 GetBE16(U32 i)  { return 0; }
U32 GetBE32(U32 i)  { return 0; }
void _Return() {
	U32 h = 0;  // HANDLE
}
void _Invoke(U32 n) {
	U32 h = 0;  // HANDLE
	U32 i = 0;  // ??
	U32 n1 = (n==3) ? 4 : 2;
}

#define UCODE(s, g)   { s, [](njThread *t){ g; } }
///
/// micro-code (built-in methods)
///
static njMethod ucode[256] = {
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
    /*10*/  UCODE("bipush",   PushI(GetB_C(t->pc++))),
    /*11*/  UCODE("sipush",   PushI(GetBE16(t->pc)); t->pc += 2),
    /*12*/  UCODE("ldc",      {}),
    /*13*/  UCODE("ldcw",     {}),
    /*14*/  UCODE("ldc2_w",   {}),
    /*15*/  UCODE("iload",    PushI(LoadI(OFF))),
    /*16*/  UCODE("lload",    PushL(LoadL(OFF))),
    /*17*/  UCODE("fload",    PushF(LoadF(OFF))),
    /*18*/  UCODE("dload",    PushD(LoadD(OFF))),
    /*19*/  UCODE("aload",    PushA(LoadA(OFF))),
    /*1A*/  UCODE("iload_0",  PushI(LoadI(0))),
    /*1B*/  UCODE("iload_1",  PushI(LoadI(1))),
    /*1C*/  UCODE("iload_2",  PushI(LoadI(2))),
    /*1D*/  UCODE("iload_3",  PushI(LoadI(3))),
    /*1E*/  UCODE("lload_0",  PushL(LoadL(0))),
    /*1F*/  UCODE("lload_1",  PushL(LoadL(1))),
    /*20*/  UCODE("lload_2",  PushL(LoadL(2))),
    /*21*/  UCODE("lload_3",  PushL(LoadL(3))),
    /*22*/  UCODE("fload_0",  PushF(LoadF(0))),
    /*23*/  UCODE("fload_1",  PushF(LoadF(1))),
    /*24*/  UCODE("fload_2",  PushF(LoadF(2))),
    /*25*/  UCODE("fload_3",  PushF(LoadF(3))),
    /*26*/  UCODE("dload_0",  PushD(LoadD(0))),
    /*27*/  UCODE("dload_1",  PushD(LoadD(1))),
    /*28*/  UCODE("dload_2",  PushD(LoadD(2))),
    /*29*/  UCODE("dload_3",  PushD(LoadD(3))),
    /*2A*/  UCODE("aload_0",  PushA(LoadA(0))),
    /*2B*/  UCODE("aload_1",  PushA(LoadA(1))),
    /*2C*/  UCODE("aload_2",  PushA(LoadA(2))),
    /*2D*/  UCODE("aload_3",  PushA(LoadA(3))),
    /*2E*/  UCODE("iaload",   S32 n = PopI(); PushI(GetI_A(PopR(), n))),
    /*2F*/  UCODE("faload",   S32 n = PopI(); PushF(GetF_A(PopR(), n))),
    /*30*/  UCODE("laload",   S32 n = PopI(); PushL(GetL_A(PopR(), n))),
    /*31*/  UCODE("daload",   S32 n = PopI(); PushD(GetD_A(PopR(), n))),
    /*32*/  UCODE("aaload",   S32 n = PopI(); PushA(GetA_A(PopR(), n))),
    /*33*/  UCODE("baload",   S32 n = PopI(); PushI(GetB_A(PopR(), n))),
    /*34*/  UCODE("caload",   S32 n = PopI(); PushI(GetC_A(PopR(), n))),
    /*35*/  UCODE("saload",   S32 n = PopI(); PushI(GetS_A(PopR(), n))),
    /// @}
    /// @definegroup Store ops (CC: TODO)
    /// @{
    /*36*/  UCODE("istore",   StorI(OFF, PopI())),
    /*37*/  UCODE("lstore",   StorL(OFF, PopL())),
    /*38*/  UCODE("fstore",   StorF(OFF, PopF())),
    /*39*/  UCODE("dstore",   StorD(OFF, PopD())),
    /*3A*/  UCODE("astore",   StorR(OFF, PopA())),
    /*3B*/  UCODE("istore_0", StorI(0, PopI())),
    /*3C*/  UCODE("istore_1", StorI(1, PopI())),
    /*3D*/  UCODE("istore_2", StorI(2, PopI())),
    /*3E*/  UCODE("istore_3", StorI(3, PopI())),
    /*3F*/  UCODE("lstore_0", StorL(0, PopL())),
    /*40*/  UCODE("lstore_1", StorL(1, PopL())),
    /*41*/  UCODE("lstore_2", StorL(2, PopL())),
    /*42*/  UCODE("lstore_3", StorL(3, PopL())),
    /*43*/  UCODE("fstore_0", StorF(0, PopF())),
    /*44*/  UCODE("fstore_1", StorF(1, PopF())),
    /*45*/  UCODE("fstore_2", StorF(2, PopF())),
    /*46*/  UCODE("fstore_3", StorF(3, PopF())),
    /*47*/  UCODE("dstore_0", StorF(0, PopD())),
    /*48*/  UCODE("dstore_1", StorF(1, PopD())),
    /*49*/  UCODE("dstore_2", StorF(2, PopD())),
    /*4A*/  UCODE("dstore_3", StorF(3, PopD())),
    /*4B*/  UCODE("astore_0", StorR(0, PopA())),
    /*4C*/  UCODE("astore_1", StorR(1, PopA())),
    /*4D*/  UCODE("astore_2", StorR(2, PopA())),
    /*4E*/  UCODE("astore_3", StorR(3, PopA())),
    /*4F*/  UCODE("iastore",  S32 v = PopI(); S32 n = PopI(); SetI_A(PopR(), n, v)),
    /*50*/  UCODE("lastore",  S64 v = PopL(); S32 n = PopI(); SetL_A(PopR(), n, v)),
    /*51*/  UCODE("fastore",  F32 v = PopF(); S32 n = PopI(); SetF_A(PopR(), n, v)),
    /*52*/  UCODE("dastore",  F64 v = PopD(); S32 n = PopI(); SetD_A(PopR(), n, v)),
    /*53*/  UCODE("aastore",  P32 v = PopA(); S32 n = PopI(); SetA_A(PopR(), n, v)),
    /*54*/  UCODE("bastore",  U8  v = PopI(); S32 n = PopI(); SetB_A(PopR(), n, v)),
    /*55*/  UCODE("castore",  U16 v = PopI(); S32 n = PopI(); SetC_A(PopR(), n, v)),
    /*56*/  UCODE("sastore",  S16 v = PopI(); S32 n = PopI(); SetS_A(PopR(), n, v)),
    /// @}
    /// @definegroup Stack ops
    /// @{
    /*57*/  UCODE("pop",      TopS32 = PopI()),
    /*58*/  UCODE("pop2",     PopI(); TopS32 = PopI()),
    /*59*/  UCODE("dup",      PushI(TopS32)),
    /*5A*/  UCODE("dup_x1",   {}),
    /*5B*/  UCODE("dup_x2",   {}),
    /*5C*/  UCODE("dup2",     PushI(t->ss[-1]); PushI(t->ss[-1])),
    /*5D*/  UCODE("dup2_x1",  {}),
    /*5E*/  UCODE("dup2_x2",  {}),
    /*5F*/  UCODE("swap",     S64 n = t->ss.pop(); PushI(n)),
    /// @}
    /// @definegroup ALU Arithmetic ops
    /// @{
    /*60*/  UCODE("iadd", TopS32 += PopI()),
    /*61*/  UCODE("ladd", TopS64 += PopL()),
    /*62*/  UCODE("fadd", TopF32 += PopF()),
    /*63*/  UCODE("dadd", TopF64 += PopD()),
    /*64*/  UCODE("isub", TopS32 = PopI() - TopS32),
    /*65*/  UCODE("lsub", TopS64 = PopL() - TopS64),
    /*66*/  UCODE("fsub", TopF32 = PopF() - TopF32),
    /*67*/  UCODE("dsub", TopF64 = PopD() - TopF64),
    /*68*/  UCODE("imul", TopS32 *= PopI()),
    /*69*/  UCODE("lmul", TopS64 *= PopL()),
    /*6A*/  UCODE("fmul", TopF32 *= PopF()),
    /*6B*/  UCODE("dmul", TopF64 *= PopD()),
    /*6C*/  UCODE("idiv", TopS32 = PopI() / TopS32),
    /*6D*/  UCODE("ldiv", TopS64 = PopL() / TopS64),
    /*6E*/  UCODE("fdiv", TopF32 = PopF() / TopF32),
    /*6F*/  UCODE("ddiv", TopF64 = PopD() / TopF64),
    /*70*/  UCODE("irem", TopS32 = PopI() % TopS32),
    /*71*/  UCODE("lrem", TopS64 = PopL() % TopS64),
    /*72*/  UCODE("frem", {}),
    /*73*/  UCODE("drem", {}),
    /*74*/  UCODE("ineg", TopS32 = -TopS32),
    /*75*/  UCODE("lneg", TopS64 = -TopS64),
    /*76*/  UCODE("fneg", TopF32 = -TopF32),
    /*77*/  UCODE("dneg", TopF64 = -TopF64),
    /// @}
    /// @definegroup ALU Logical ops
    /// @{
    /*78*/  UCODE("ishl", TopS32 <<= PopI()),
    /*79*/  UCODE("lshl", TopS64 <<= PopL()),
    /*7A*/  UCODE("ishr", TopS32 >>= PopI()),
    /*7B*/  UCODE("lshr", TopS64 >>= PopL()),
    /*7C*/  UCODE("iushr",TopU32 >>= PopI()),
    /*7D*/  UCODE("lushr",TopU64 >>= PopL()),
    /*7E*/  UCODE("iand", TopU32 = PopI() & TopU32),
    /*7F*/  UCODE("land", TopU64 = PopL() & TopU64),
    /*80*/  UCODE("ior",  TopU32 = PopI() | TopU32),
    /*81*/  UCODE("lor",  TopU64 = PopL() | TopU64),
    /*82*/  UCODE("ixor", TopU32 = PopI() ^ TopU32),
    /*83*/  UCODE("lxor", TopU64 = PopL() ^ TopU64),
    /*84*/  UCODE("iinc", {}),
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
    /*99*/  UCODE("ifeq", Jump(TopS32 == 0)),
    /*9A*/  UCODE("ifne", Jump(TopS32 != 0)),
    /*9B*/  UCODE("iflt", Jump(TopS32 <  0)),
    /*9C*/  UCODE("ifge", Jump(TopS32 >= 0)),
    /*9D*/  UCODE("ifgt", Jump(TopS32 >  0)),
    /*9E*/  UCODE("ifle", Jump(TopS32 <= 0)),
    /*9F*/  UCODE("if_icmpeq", Jump(t->ss.pop() == TopS32)),
    /*A0*/  UCODE("if_icmpne", Jump(t->ss.pop() != TopS32)),
    /*A1*/  UCODE("if_icmplt", Jump(t->ss.pop() <  TopS32)),
    /*A2*/  UCODE("if_icmpge", Jump(t->ss.pop() >= TopS32)),
    /*A3*/  UCODE("if_icmpgt", Jump(t->ss.pop() >  TopS32)),
    /*A4*/  UCODE("if_icmple", Jump(t->ss.pop() <= TopS32)),
    /*A5*/  UCODE("if_acmpeq", {}),
    /*A6*/  UCODE("if_acmpne", {}),
    /// @}
    /// @definegroup Branching ops
    /// @{
    /*A7*/  UCODE("goto",      Jump(true)),
    /*A8*/  UCODE("jsr",       PushI(t->pc + 2); Jump(true)),
    /*A9*/  UCODE("ret",       t->pc = LoadI(OFF)),
    /*AA*/  UCODE("tableswitch",  {}),
    /*AB*/  UCODE("lookupswitch", {}),
    /// @}
    /// @definegroup Return ops
    /// @{
    /*AC*/  UCODE("ireturn",   S32 n = TopS32; Ret(); PushI(n)),
    /*AD*/  UCODE("lreturn",   S64 n = TopS64; Ret(); PushL(n)),
    /*AE*/  UCODE("freturn",   F32 n = TopF32; Ret(); PushF(n)),
    /*AF*/  UCODE("dreturn",   F64 n = TopF64; Ret(); PushD(n)),
    /*B0*/  UCODE("areturn",   P32 n = TopU32; Ret(); PushA(n)),
    /*B1*/  UCODE("return",    Ret()),
    /// @}
    /// @definegroup Field Fetch ops
    /// @{
    /*B2*/  UCODE("getstatic", GetI_S(GetBE16(t->pc));    t->pc+=2),
    /*B3*/  UCODE("putstatic", SetI_S(GetBE16(t->pc), 1); t->pc+=2),
    /*B4*/  UCODE("getfield",  GetI_F(GetBE16(t->pc));    t->pc+=2),
    /*B5*/  UCODE("putfield",  SetI_F(GetBE16(t->pc), 3); t->pc+=2),
    /// @}
    /// @definegroup Method/Interface Invokation ops
    /// @{
    /*B6*/  UCODE("invokevirtual",   Nest(0)),
    /*B7*/  UCODE("invokespecial",   Nest(1)),
    /*B8*/  UCODE("invokestatic",    Nest(2)),
    /*B9*/  UCODE("invokeinterface", Nest(3)),
    /*BA*/  UCODE("invokedynamic",   Nest(4)),
    /// @}
    /// @definegroup New and Array ops
    /// @{
    /*BB*/  UCODE("new",          {}),
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
    /*C4*/  UCODE("wide",         t->wide = true),
    /*C5*/  UCODE("multianewarray", {}),
    /*C6*/  UCODE("ifnull",       Jump(PopA() == 0)),
    /*C7*/  UCODE("ifnonnull",    Jump(PopA() != 0)),
    /*C8*/  UCODE("goto_w",       t->pc += GetBE32(t->pc) - 1),
    /*C9*/  UCODE("jsr_w",        PushI(t->pc + 4); t->pc += GetBE32(t->pc) - 1)
};

int main(int ac, char** av) {
	njClass c(ucode);
	return 0;
}
