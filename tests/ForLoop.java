import nanojvm.Forth;

class ForLoop
{
    public static void clock() {
        int v0 = Forth.clock();         // b8 00 02 3b
        int v1 = 0;                     // 03 3c
        for (int i=0; i<10; i++) {      // 03 3d 1c 10 0a a2 00 11
            Forth.delay(50);            // 10 32 b8 00 03
            v1 -= 1;                    // 84 01 ff 
        }                               // 84 02 01 a7 ff ef
        v0 = Forth.clock() - v0;        // b8 00 02 1a 64 3b
    }                                   // b1
    
    public static void main() {
        Forth.interpreter();            // b8 00 04
    }                                   // b1
}
