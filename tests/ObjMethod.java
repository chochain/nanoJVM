import ej32.Forth;

class ObjMethod
{
    static int cv0;                     // cv[07]
    int    iv0;                         // iv[03]
    
    public int clock() {
        int v0 = Forth.clock();         // b8 00 02 3c
        return iv0 = v0;                // 2a 1b 5a b5 00 03 ac
    }
    
    public static void main(String[] av) {
        ObjMethod o = new ObjMethod();  // bb 00 04 59 b7 00 05 4b
        o.iv0 = o.clock();              // 2a 2a b6 00 06 b5 00 03
        Forth.words();                  // b8 00 07
        cv0 = o.iv0 - o.clock();        // 2a b4 00 03 2a b6 00 06 64 b3 00 08
    }                                   // b1
}
