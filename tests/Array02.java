class Array02
{
    public static void main(String[] av) {     // 2a b7 00 01 b1
        int a[][] = {                  // 05 bd 00 02
            {2,12,22},                 // 59 03 06 bc 0a 59 05 10 0c 4f 59 05 10 16 4f 53
            {44,55,66}                 // 59 03 10 2c 4f 59 03 10 37 4f 59 04 10 42 4f 53
        };

        System.out.println(a.length);  // 4c b2 00 03 2b be b6 00 04
        for (int i=0; i<2; i++) {      // 03 3d 1c 05 a2 00 21 03 3e
            for (int j=0; j<3; j++) {
                System.out.println(a[i][j]);  // 2b 1c 32 1d 2e b6 00 04
            }                          // 84 03 01 a7 ff ed 
        }                              // 84 02 01 a7 ff e0
    }                                  // b1
}
