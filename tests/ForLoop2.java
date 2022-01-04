import ej32.Forth;

class ForLoop2
{
    public static void main(String[] av) {
        int v0 = Forth.clock();         // b8 00 02 3b
        for (int i=11; i<13; i++) {       // 03 3d 1c 07 a2 00 11
            for (int j=1; j<4; j++) {
                Forth.delay(100);       // 10 64 b8 00 03
                System.out.print(i);
                System.out.print(j);
            }
        }
        v0 = Forth.clock() - v0;        // b8 00 02 1a 64 3b
    }                                   // b1
}
