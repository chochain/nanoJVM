import nanojvm.Forth;

class Hello
{
    static int cls_v0;                  // global[05]
    int inst_v0;
    int inst_v1;
    
    public static void main()
    {
        int main_v0;                    // ss[0]
        
        Forth.words();                  // b8 00 02
        main_v0 = Forth.clock();        // b8 00 03 3b
        cls_v0  = Forth.here() - 0x100; // b8 00 04 11 01 00 64 b3 00 05
        Forth.dump(cls_v0, 0x100);      // b2 00 05 11 01 00 b8 00 06
        main_v0 -= Forth.clock();       // 1a b8 00 03 64 3b
        cls_v0  = -main_v0;             // 1a 74 b3 00 05
        Forth.ss();                     // b8 00 07
//        main_v0 = Forth.tick("words");  // 12 08 b8 00 09 3b
    }                                   // b1
}
