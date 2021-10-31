import nanojvm.Forth;

class InstVar
{
    static int cv0;                // global[05]
    int iv0;
    int iv1;
    
    public static void main()
    {
        int v0;                         // ss[0]
        InstVar h = new InstVar();      // bb 00 02 59 b7 00 03 4c 2b
        
        h.iv1 = Forth.clock();          // b8 00 04 b5 00 05
        cv0   = Forth.here();           // b8 00 06 b3 00 07
        h.iv1 -= Forth.clock();         // 2b 59 b4 00 05 b8 00 04 64 b5 00 05
        cv0   = h.iv0 = -h.iv1;         // 2b 2b b4 00 05 74 5a b5 00 08 b3 00 07
    }                                   // b1
}
