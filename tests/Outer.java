import ej32.Forth;

class Outer
{
    static int cv0;                     // global[05]
    int iv0;
    int iv1;

    public static void clock() {
        int v0 = Forth.here() - 0x100;  // b8 00 02 11 01 00 64 3b
        Forth.dump(v0, 0x100);          // 1a 11 00 01 b8 00 03
        cv0  = Forth.clock();           // b8 00 04 b3 00 05 11 01 f4
        Forth.delay(500);               // b8 00 06
        cv0  = Forth.clock() - cv0;     // b8 00 04 b2 00 05 64 b3 00 05
    }                                   // b1
    
    public static void main() {
        Forth.interpreter();            // b8 00 07
    }                                   // b1
}
