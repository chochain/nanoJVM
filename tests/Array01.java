class Array01
{
    public static void main(String[] av) {     // 2a b7 00 01 b1
        int a[] =                      // 06 bc 0a
            {2,12,22};                 // 59 03 05 4f 59 04 10 0c 4f 59 05 10 16 4f

        System.out.println(a.length);  // 4c b2 00 02 2b be b6 00 03 b2 00 02
        System.out.println(a[0]);      // 2b be b6 00 03 b2 00 02
        System.out.println(a[1]);      // 2b 04 2e b6 00 03 b2 00 02
        System.out.println(a[2]);      // 2b 05 2e b6 00 03
    }                                  // b1
}
