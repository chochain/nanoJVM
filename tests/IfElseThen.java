import ej32.Forth;

class IfElseThen
{
    public static void main(String[] av)
    {
        int v0;                   // xs[0]
        int v1 = 0;               // 18 03 3c
        int v2;
        int v3;
        int v4 = 0x7fff;          // 11 7f ff 36 04 15 04
        
        if (v4 > v1) {            // 1b a4 00 0a
            Forth.clock();        // b8 00 02 57 a7 00 06
        }
        else {
            Forth.ss();           // b8 00 03
        }
    }                             // b1
}
