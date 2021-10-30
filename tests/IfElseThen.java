import nanojvm.Forth;

class IfElseThen
{
    public static void main()
    {
        int main_v0 = 1;                // ss[0]
        
        if (main_v0 > 0) {
            main_v0 -= 1;
        }
        else {
            main_v0 += 1;
        }
    }                                   // b1
}
