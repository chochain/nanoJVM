#ifndef NANOJVM_CORE_H
#define NANOJVM_CORE_H
#include <stdint.h>		// int16_t, ...
#include <stdlib.h>     // strtol
#include <string.h>		// strcmp
using namespace std;
///
/// array class template (so we don't have dependency on C++ STL)
/// Note:
///   * using decorator pattern
///   * this is similar to vector class but much simplified
///   * v array is dynamically allocated due to ESP32 has a 96K hard limit
///
#define RANGE_CHECK
//#define ENDL            endl; fout_cb(fout.str().length(), fout.str().c_str()); fout.str("")
#define ENDL            endl
#define yield()

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
    T push(T t) {
        if (idx<N) return v[max=idx++] = t;
        throw "ERR: List full";
    }
#else
    T pop()     { return v[--idx]; }
    T push(T t) { return v[max=idx++] = t; }
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

typedef U16         IU;
typedef S32         DU;
///
/// alignment macros
///
#define ALIGN(sz)       ((sz) + (-(sz) & 0x1))
#define ALIGN16(sz)     ((sz) + (-(sz) & 0xf))
///
/// method class
///
struct Thread;
typedef void (*fop)(Thread&);
struct Method {
    const char *name = 0;
    union {
        fop xt = 0;
        struct {
            U32  def:    1;  // 0:native, 1:composite
            U32  immd:   1;
            U32  acc:    2;  // public, private, protected
            U32  type:   4;  // static, finall, virtual
            U32  pfa:   24;
        };
    };
    Method(const char *n, fop f, bool im=false) : name(n), xt(f) {
        immd = im ? 1 : 0;
    }
    Method() {}
};
///
/// class, thread, and method classes
///
struct Thread {
    DU    base = 10;
    bool  compile;          /// compile flag
    bool  wide;             /// wide flag
    DU    tos;              /// top of stack
    IU    WP;
    U8    *IP;              /// instruction pointer (program counter)
    
    List<DU, 256>  ss;      /// data stack
    int   local;            /// local stack index
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
    void ret()             { IP = 0; }
    void jmp()             { IP += *(IU*)IP - 1;   }
    void cjmp(bool f)      { IP += f ? *(IU*)IP - 1 : sizeof(IU); }
    ///
    /// local parameter access, CC:TODO
    ///
    template<typename T>
    T    load(U32 i, T n)  { return *(T*)&ss[i+local]; }
    template<typename T>
    void store(U32 i, T n) { *(T*)&ss[i+local] = n; }
};

#define STRLEN(s) (ALIGN(strlen(s)+1))      /** calculate string size with alignment     */

struct Klass {
    const char 	       *name;               /// class name
    List<Method, 1024> dict;                /// class methods
    List<U8, 1024*48>  &pmem;               /// heap pointer

    Klass(const char *n, List<U8, 1024*48> &heap) : name(n), pmem(heap) {}

    int find(const char *s) {
        for (int i = dict.idx -1; i>=0; --i) {
            if (strcmp(s, dict[i].name)==0) return i | 0xf000;
        }
        return -1;
    }
	int  handle_number(Thread &t, const char *idiom);
    ///
    /// compiler methods
    ///
    void add_iu(IU i) { pmem.push((U8*)&i, sizeof(IU)); }
    void add_du(DU v) { pmem.push((U8*)&v, sizeof(DU)); }
    void add_str(const char *s) { int sz = STRLEN(s); pmem.push((U8*)s,  sz); }

    void colon(const char *name);
};
#endif // NANOJVM_CORE_H
