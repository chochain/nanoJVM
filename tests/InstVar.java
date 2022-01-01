import ej32.Forth;

class InstVar
{
    static int cv0;                     // [07]
    int iv0;                            // [09]
    int iv1;                            // [05]
    
    public static void main(String[] av)
    {
        int v0;                         // ss[0]
        InstVar h = new InstVar();      // bb 00 02 59 b7 00 03 4c 2b
        
        h.iv1 = Forth.clock();          // b8 00 04 b5 00 05
        cv0   = Forth.here();           // b8 00 06 b3 00 07
        Forth.delay(500);               // 11 01 f4 b8 00 08
        h.iv1 -= Forth.clock();         // 2b 59 b4 00 05 b8 00 04 64 b5 00 05
        v0    = h.iv0 = -h.iv1;         // 2b 2b b4 00 05 74 5a b5 00 09 3b
        cv0   = v0;                     // 1a b3 00 07
    }                                   // b1
}
