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
typedef U32         PU;
///
/// memory alignment macros
///
#define ALIGN(sz)   ((sz) + (-(sz) & 0x1))
#define ALIGN16(sz) ((sz) + (-(sz) & 0xf))
#define STRLEN(s)   (ALIGN(strlen(s)+1))    /** calculate string size with alignment */
///
/// Thread class
///
struct Thread {
    List<DU, SS_SZ>  ss;    /// data stack
    int   local;            /// local stack index

    bool  compile = false;  /// compile flag
    bool  wide    = false;  /// wide flag
    DU    base    = 10;     /// radix
    DU    tos     = -1;     /// top of stack
    IU    WP      = 0;      /// method index
    U8    *IP     = NULL;   /// instruction pointer (program counter)
    U8    *M0     = NULL;	/// cached base address of memory pool

    Thread(U8 *mem) : M0(mem) {}
    ///
    /// opcode fetcher
    ///
    U8   getBE8()          { return *IP++; }
    U16  getBE16(U16 i=0)  { U16 n = *(U16*)IP; IP += sizeof(U16); return n; }
    ///
    /// stack ops
    ///
    void push(DU v)        { ss.push(tos); tos = v; }
    DU   pop()             { DU n = tos; tos = ss.pop(); return n; }
    ///
    /// branching ops
    ///
    void invoke(U16 i)     { /* TODO */ }
    void ret()             { IP = NULL; }
    void jmp()             { IP += *(PU*)IP - 1;   }
    void cjmp(bool f)      { IP += f ? *(PU*)IP - 1 : sizeof(PU); }
    ///
    /// local parameter access, CC:TODO
    ///
    template<typename T>
    T    load(U32 i, T n)  { return *(T*)&ss[i+local]; }
    template<typename T>
    void store(U32 i, T n) { *(T*)&ss[i+local] = n; }
};
typedef void (*fop)(Thread&); /// opcode function pointer
///
/// Method class
///
struct Method {
    const char *name = 0;     /// for debugging, TODO (in const_pool)
    union {
        fop xt = 0;
        struct {
            U8   def:    1;   /// 0:native, 1:composite
            U8   immd:   1;
            U8   acc:    2;   /// public, private, protected
            U8   type:   4;   /// static, finall, virtual
            U8   cid;         /// class index
            IU   pfa;         /// method offset to pmem
        };
    };
    Method(const char *n, fop f, bool im=false) : name(n), xt(f) {
        immd = im ? 1 : 0;
    }
    Method() {}

    void exec(Thread &t) { (*(fop)(((uintptr_t)xt)&~0x3))(t); }
};
#endif // NANOJVM_CORE_H
