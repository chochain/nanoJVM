class Array02
{
    public static void main(String[] av) {     // 2a b7 00 01 b1
        int a[][] = {                  // 05 bd 00 02 59
            {1,2,3},                   // 59 03 06 bc 0a 59 03 04 06 6 bc 0a
            {4,5,6}
        };

        System.out.println(a.length);  // 4c b2 00 02 2b be b6 00 03 b2 00 02
        for (int i=0; i<2; i++) {
            for (int j=0; j<3; j++) {
                System.out.println(a[i][j]);      // 2b be b6 00 03 b2 00 02
            }
        }
    }                                  // b1
}
