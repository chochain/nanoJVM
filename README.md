# nanoJVM
## JVM for microcontroller
* work in progress

### docs
### src
* core   - thread class functions
* forth  - forth dictionary
* jvm    - java virtual machine
* loader - bytecode loader 
* mmu    - memory management unit
* ucode  - microcode unit
* main   - main module
### tests
Use the following toolchain to produce bytecode (and analysis)
> javac -g:none Hello.java
> javap -c -p Hello.class
> od -A x -t x1z -v Hello.class
|case|source|note|
|---|---|---|
|Hello|
```
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
```||
|IfElesThen|
```
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
```|
|ForLoop||
|InstVar||
|NObj||
|ObjMethod||
|Outer||
|Print||
|ESP32Test||
Example:
```
/home/gnii/devel/java/nanojvm/tests% ../Debug/nanojvm Hello.class

Java class file: Hello.class
0000: ca fe ba be  00 00 00 37  00 24 0a 00  09 00 12 0a  _______7_$______
0010: 00 13 00 14  0a 00 13 00  15 0a 00 13  00 16 09 00  ________________
0020: 08 00 17 0a  00 13 00 18  0a 00 13 00  19 07 00 1a  ________________
0030: 07 00 1b 01  00 06 63 6c  73 5f 76 30  01 00 01 49  ______cls_v0___I
0040: 01 00 07 69  6e 73 74 5f  76 30 01 00  07 69 6e 73  ___inst_v0___ins
0050: 74 5f 76 31  01 00 06 3c  69 6e 69 74  3e 01 00 03  t_v1___<init>___
0060: 28 29 56 01  00 04 43 6f  64 65 01 00  04 6d 61 69  ()V___Code___mai
0070: 6e 0c 00 0e  00 0f 07 00  1c 0c 00 1d  00 0f 0c 00  n_______________
0080: 1e 00 1f 0c  00 20 00 1f  0c 00 0a 00  0b 0c 00 21  _____ _________!
0090: 00 22 0c 00  23 00 0f 01  00 05 48 65  6c 6c 6f 01  _"__#_____Hello_
00a0: 00 10 6a 61  76 61 2f 6c  61 6e 67 2f  4f 62 6a 65  __java/lang/Obje
00b0: 63 74 01 00  0a 65 6a 33  32 2f 46 6f  72 74 68 01  ct___ej32/Forth_
00c0: 00 05 77 6f  72 64 73 01  00 05 63 6c  6f 63 6b 01  __words___clock_
00d0: 00 03 28 29  49 01 00 04  68 65 72 65  01 00 04 64  __()I___here___d
00e0: 75 6d 70 01  00 05 28 49  49 29 56 01  00 02 73 73  ump___(II)V___ss
00f0: 00 20 00 08  00 09 00 00  00 03 00 08  00 0a 00 0b  _ ______________
0100: 00 00 00 00  00 0c 00 0b  00 00 00 00  00 0d 00 0b  ________________
0110: 00 00 00 02  00 00 00 0e  00 0f 00 01  00 10 00 00  ________________
0120: 00 11 00 01  00 01 00 00  00 05 2a b7  00 01 b1 00  __________*_____
0130: 00 00 00 00  09 00 11 00  0f 00 01 00  10 00 00 00  ________________
0140: 35 00 02 00  01 00 00 00  29 b8 00 02  b8 00 03 3b  5_______)______;
0150: b8 00 04 11  01 00 64 b3  00 05 b2 00  05 11 01 00  ______d_________
0160: b8 00 06 1a  b8 00 03 64  3b 1a 74 b3  00 05 b8 00  _______d;_t_____
0170: 07 b1 00 00  00 00 00 00  ff ff ff ff  ff ff ff ff  ________________
[01]000a:0a=>[9,12]
[02]000f:0a=>[13,14]
[03]0014:0a=>[13,15]
[04]0019:0a=>[13,16]
[05]001e:09=>[8,17]
[06]0023:0a=>[13,18]
[07]0028:0a=>[13,19]
[08]002d:07=>1a
[09]0030:07=>1b
[0a]0033:01=>cls_v0
[0b]003c:01=>I
[0c]0040:01=>inst_v0
[0d]004a:01=>inst_v1
[0e]0054:01=><init>
[0f]005d:01=>()V
[10]0063:01=>Code
[11]006a:01=>main
[12]0071:0c=>[e,f]
[13]0076:07=>1c
[14]0079:0c=>[1d,f]
[15]007e:0c=>[1e,1f]
[16]0083:0c=>[20,1f]
[17]0088:0c=>[a,b]
[18]008d:0c=>[21,22]
[19]0092:0c=>[23,f]
[1a]0097:01=>Hello
[1b]009f:01=>java/lang/Object
[1c]00b2:01=>ej32/Forth
[1d]00bf:01=>words
[1e]00c7:01=>clock
[1f]00cf:01=>()I
[20]00d5:01=>here
[21]00dc:01=>dump
[22]00e3:01=>(II)V
[23]00eb:01=>ss
class [8]Hello : [9]java/lang/Object
  p_intf=f8, p_attr=fa
  sz_cls=4, sz_inst=8
  n_method=2, p_method=114
  [0e]<init>()V (5 bytes)
  [11]main()V (29 bytes)
} loaded.

eJ32 v1 staring...

main() <-1|0> ok
j0149:b8 invokestatic ej32/Forth.words:()V $0
Hello : java/lang/Object 15a0
	main 1590 <init> 157e 
ej32/ESP32 : ej32/Forth 1564
	ledcWriteTone 1548 ledcSetup 1530 ledcAttachPin 1514 analogWrite 14fa analogRead 14e0 digitalWrite 14c4 digitalRead 14aa pinMode 1494 
ej32/Forth : java/lang/Object 147a
	bye 1468 interpreter 144e delay 143a clock 1426 tick 1412 dump 13fe ss 13ec words 13d8 here 13c4 hex 13b2 
	decimal 139c +! 138a allot 1376 , 1366 ! 1356 @ 1346 ; 1336 : 1326 constant 130e variable 12f6 
	create 12e0 unnest 12ca dostr 12b6 dolit 12a2 dovar 128e 
java/io/PrintStream : java/lang/Object 126c
	println 1256 println 1240 print 122c print 1218 
java/lang/System : java/lang/Object 11ec
	<init> 11d6 
java/lang/String : java/lang/Object 11aa
	<init> 1194 
java/lang/Object : Ucode 1174
	<init> 115e 
 <-1|0> ok
j014c:b8 invokestatic ej32/Forth.clock:()I $1 <-1|0 485093865> ok
j014f:3b istore_0 <-1|485093865> ok
j0150:b8 invokestatic ej32/Forth.here:()I $2 <-1|485093865 5560> ok
j0153:11 sipush <-1|485093865 5560 256> ok
j0156:64 isub <-1|485093865 5304> ok
j0157:b3 putstatic Hello.cls_v0:I $0 <-1|485093865> ok
j015a:b2 getstatic $0 <-1|485093865 5304> ok
j015d:11 sipush <-1|485093865 5304 256> ok
j0160:b8 invokestatic ej32/Forth.dump:(II)V $3
m14c0: 00 00 00 00  aa 14 0e 00  64 69 67 69  74 61 6c 57  ________digitalW
m14d0: 72 69 74 65  00 61 55 03  2c 70 e3 55  00 00 00 00  rite_aU_,p_U____
m14e0: c4 14 0c 00  61 6e 61 6c  6f 67 52 65  61 64 00 61  ____analogRead_a
m14f0: c1 03 2c 70  e3 55 00 00  00 00 e0 14  0c 00 61 6e  __,p_U________an
m1500: 61 6c 6f 67  57 72 69 74  65 00 23 04  2c 70 e3 55  alogWrite_#_,p_U
m1510: 00 00 00 00  fa 14 0e 00  6c 65 64 63  41 74 74 61  ________ledcAtta
m1520: 63 68 50 69  6e 00 85 04  2c 70 e3 55  00 00 00 00  chPin___,p_U____
m1530: 14 15 0a 00  6c 65 64 63  53 65 74 75  70 00 f6 04  ____ledcSetup___
m1540: 2c 70 e3 55  00 00 00 00  30 15 0e 00  6c 65 64 63  ,p_U____0___ledc
m1550: 57 72 69 74  65 54 6f 6e  65 00 59 05  2c 70 e3 55  WriteTone_Y_,p_U
m1560: 00 00 00 00  7a 14 0c 00  65 6a 33 32  2f 45 53 50  ____z___ej32/ESP
m1570: 33 32 00 0a  7a 14 00 00  48 15 00 00  00 00 00 00  32__z___H_______
m1580: 08 04 3c 69  6e 69 74 3e  00 00 2a 01  00 00 00 00  __<init>__*_____
m1590: 7e 15 06 04  6d 61 69 6e  00 3e 49 01  00 00 00 00  ~___main_>I_____
m15a0: 64 15 06 00  48 65 6c 6c  6f 00 74 11  00 00 90 15  d___Hello_t_____
m15b0: 04 00 08 00  b8 14 00 00  00 00 00 00  00 00 00 00  ________________
m15c0: 00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ________________
 <-1|485093865> ok
j0163:1a iload_0 <-1|485093865 485093865> ok
j0164:b8 invokestatic $1 clock <-1|485093865 485093865 485093866> ok
j0167:64 isub <-1|485093865 -1> ok
j0168:3b istore_0 <-1|-1> ok
j0169:1a iload_0 <-1|-1 -1> ok
j016a:74 ineg <-1|-1 1> ok
j016b:b3 putstatic $0 <-1|-1> ok
j016e:b8 invokestatic ej32/Forth.ss:()V $4 <-1|-1> ok
 <-1|-1> ok
j0171:b1 return

eJ32 done.
```
