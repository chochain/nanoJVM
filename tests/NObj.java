import ej32.Forth;

class NObj
{
    int iv0;                            // [09]
    int iv1;                            // [05]

    NObj(int i, int j) {
        int x = i + j;                  // 2a b7 00 01 1b 1c 60 3e
        iv0 = i;                        // 2a 1b b5 00 02
        iv1 = j;                        // 2a 1c b5 00 03
    }                                   // b1
    
    public static void main(String[] av)
    {
        NObj o0 = new NObj(1, 3);       // bb 00 04 59 04 06 b7 00 05 4c
        NObj o1 = new NObj(22, 33);     // bb 00 04 59 10 16 10 21 b7 00 05 4d 2b
        
        o0.iv1 = Forth.clock();         // b8 00 06 b5 00 03 2b
        o1.iv1 = Forth.here();          // b8 00 07 b5 00 03
        Forth.delay(500);               // 11 01 f4 b8 00 08
        o0.iv1 -= Forth.clock();        // 2a 59 b4 00 03 b8 00 06 64 b5 00 03
        o1.iv1 += -o0.iv1;              // 2b 4d 2c 2c b4 00 03 74 60 5a b5 00 03 57
    }                                   // b1
}
