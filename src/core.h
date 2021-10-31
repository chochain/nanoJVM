#ifndef NANOJVM_CORE_H
#define NANOJVM_CORE_H
#include <stdint.h>     // int16_t, ...
#include <stdlib.h>     // strtol
#include <string.h>     // strcmp
using namespace std;
///
/// conditional compilation options
///
#define RANGE_CHECK
///
/// memory block size setting
///
#define HEAP_SZ         1024*48     /** global heap space size     */
#define VT_SZ           512         /** virtual table pool size    */
#define RS_SZ           128         /** return stack size per VM   */
#define SS_SZ           256         /** data stack size per thread */
#define CONST_SZ        128         /** constant pool size         */
///
/// Arduino support macros
///
#if ARDUINO
#include <Arduino.h>
#if ESP32
#define analogWrite(c,v,mx) ledcWrite((c),(8191/mx)*min((int)(v),mx))
#endif // ESP32
#else
#include <chrono>
#include <thread>
#define millis()        chrono::duration_cast<chrono::milliseconds>( \
                            chrono::steady_clock::now().time_since_epoch()).count()
#define delay(ms)       this_thread::sleep_for(chrono::milliseconds(ms))
#define yield()         this_thread::yield()
#define PROGMEM
#endif // ARDUINO
///
/// array class template (so we don't have dependency on C++ STL)
/// Note:
///   * using decorator pattern
///   * this is similar to vector class but much simplified
///   * v array is dynamically allocated due to ESP32 has a 96K hard limit
///
template<class T, int N>
struct List {
    T   *v;             /// fixed-size array storage
    int idx = 0;        /// current index of array
    int max = 0;        /// high watermark for debugging

    List()  { v = new T[N]; }      /// dynamically allocate array memory
    ~List() { delete[] v;   }      /// free the memory
    T& operator[](int i)   { return i < 0 ? v[idx + i] : v[i]; }
#ifdef RANGE_CHECK
    T pop() {
        if (idx>0) return v[--idx];
        throw "ERR: List empty";
    }
    int push(T t) {
        if (idx<N) { v[max=idx] = t; return idx++; }
        throw "ERR: List full";
    }
#else
    T   pop()     { return v[--idx]; }
    int push(T t) { v[max=idx] = t; return idx++; }
#endif // RANGE_CHECK
    void push(T *a, int n)  { for (int i=0; i<n; i++) push(*(a+i)); }
    void merge(List& a)     { for (int i=0; i<a.idx; i++) push(a[i]);}
    void clear(int i=0)     { idx=i; }
};
///
/// universal types
///
typedef int8_t      S8;
typedef int16_t     S16;
typedef int32_t     S32;
typedef int64_t     S64;
typedef uint8_t     U8;
typedef uint16_t    U16;
typedef uint32_t    U32;
typedef uint64_t    U64;
typedef float       F32;
typedef double      F64;
typedef uintptr_t   P32;
///
/// logical size: instruction, data, and pointer units
///
typedef U16         IU;
typedef S32         DU;
typedef P32         PU;
///
/// memory alignment macros
///
#define ALIGN(sz)   ((sz) + (-(sz) & 0x1))  /** 2-byte alignment  */
#define ALIGN16(sz) ((sz) + (-(sz) & 0xf))  /** 16-byte alignment */
#define STRLEN(s)   (ALIGN(strlen(s)+1))    /** calculate string size with alignment */
///
/// Thread class
///
struct Thread {
    List<DU, SS_SZ>  ss;    /// data stack
    DU    gl[16];           /// DEBUG: class variable (static)
    DU    xs[SS_SZ];        /// DEBUG: execution local stack, REFACTOR: combine with ss
    int   local   = 0;      /// local stack index

    bool  compile = false;  /// compile flag
    bool  wide    = false;  /// wide flag
    DU    base    = 10;     /// radix
    DU    tos     = -1;     /// top of stack
    IU    WP      = 0;      /// method index
    U8    *IP     = NULL;   /// instruction pointer (program counter)
    U8    *M0     = NULL;   /// cached base address of memory pool
    IU    PC;               /// DEBUG: for loader

    Thread(U8 *heap) : M0(heap) {}
    ///
    /// opcode fetcher
    ///
    U8   getBE8();  //         { return *IP++; PC++; }
    U16  getBE16(); //         { U16 n = *(U16*)IP; IP += sizeof(U16); PC+=sizeof(U16); return n; }
    U32  getBE32(); //         { U16 n = *(U16*)IP; IP += sizeof(U16); PC+=sizeof(U16); return n; }
    ///
    /// stack ops
    ///
    void push(DU v)        { ss.push(tos); tos = v; }
    DU   pop()             { DU n = tos; tos = ss.pop(); return n; }
    ///
    /// branching ops
    ///
    void invoke(U16 itype);
    void ret()             { IP = NULL; PC = 0xffff; }
    void jmp();        //      { IP += *(PU*)IP - 1;   }
    void cjmp(bool f); //      { IP += f ? *(PU*)IP - 1 : sizeof(PU); }
    ///
    /// local parameter access, CC:TODO
    ///
    template<typename T>
    T    load(U32 i, T n)  { return *(T*)&xs[local+i]; }
    template<typename T>
    void store(U32 i, T n) { *(T*)&xs[local+i] = n; }
};
typedef void (*fop)(Thread&); /// opcode function pointer
///
/// Method class
///
struct Method {
    const char *name = 0;     /// for debugging, TODO (in const_pool)
#if METHOD_PACKED
    union {
        fop   xt = 0;         /// function pointer (or decayed lambda)
        struct {
            U32 def:  1;
            U32 immd: 1;
            U32 rsv:  30;
        };
    };
#else
    fop   xt = 0;            /// function pointer (or decayed lambda)
    U16   immd;
#endif
    Method(const char *n, fop f, bool im=false) : name(n), xt(f) {
        if (im) immd = 1;
    }
};
///
/// Word - shared struct for Class and Method
///   class list - linked list of words, dict[cls_root], pfa => next_class
///   vtable     - linked list of words, dict[class.pfa], pfa => next_method
///
#define CLS_SUPER 		0
#define CLS_INTF  		2
#define CLS_VT    		4
#define CLS_CVSZ  		6
#define CLS_IVSZ  		8
struct Word {				 /// 4-byte header
    IU  lfa;                 /// link field to previous word
    U8  len;                 /// name of method

    U8  def:    1;           /// 0:native, 1:composite
    U8  immd:   1;           /// Forth immediate word
    U8  access: 2;           /// public, private, protected
    U8  ftype:  4;           /// static, finall, virtual, synchronized

    U8  data[];              /// name field + parameter field

    char *nfa()         { return (char*)&data[0];  }
    U8   *pfa(U8 off=0) { return &data[len + off]; }
};
///
/// Klass class (same as Ruby, to avoid compiler confusion)
///
struct NativeKlass {
    const  char* name;
    U16    cvsz;
    U16    ivsz;
    U16    vtsz;
    Method vt[];
};
struct Klass {
	IU  lfa;				 /// class linked list
	IU  len;                 /// class name string length
	IU  supr;                /// index to super class
    IU  intf;                /// index to interface
    IU  vt;                  /// index to method table
    IU  cvsz;                /// size of class variables
    IU  ivsz;                /// size of instance variables
    U8  data[];              /// raw data (string name)

    char *nfa() { return (char*)&data[0];   }
    U8   *pfa() { return &data[len]; 		}
};
#endif // NANOJVM_CORE_H
