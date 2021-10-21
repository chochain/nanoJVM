#include <stdint.h>
///
/// array class template (so we don't have dependency on C++ STL)
/// Note:
///   * using decorator pattern
///   * this is similar to vector class but much simplified
///   * v array is dynamically allocated due to ESP32 has a 96K hard limit
///
#define RANGE_CHECK
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
///
/// class, thread, and method classes
///
struct njThread {
    U8    *ip;
    U32   pc;
    bool  compile;
    bool  wide;
    S32   top;

    List<S32, 256>  ss;
    List<S32, 256>  rs;
    List<S32, 128>  local;  // CC:TODO

    void push(S32 v)       { ss.push(top); top = v; }
    S32  pop()             { S32 n=top; top=ss.pop(); return n; }
    
    template<typename T>
    T    load(U32 i, T n)  { return *(T*)&local[i]; }
    template<typename T>
    void store(U32 i, T n) { *(T*)&local[i] = n; }
};
typedef void (*fop)(njThread*);
struct njMethod {
    const char *name = 0;
    union {
        fop xt = 0;
        struct {
            U8  native: 1;
            U8  immd:   1;
            U8  acc:    2;  // public, private, protected
            U8  type:   4;  // static, final, synchronized
            U8  rsv;        // reserved
            U16 ref;
        };
    };
    njMethod(const char *n, fop f, bool im=false) : name(n), xt(f) {
        immd = im ? 1 : 0;
    }
    njMethod() {}
};
struct njClass {
    const char *name;
    List<njMethod, 1024>  dict;

    njClass(njMethod ucode[256]) { for (int i=0; i<256; i++) dict.push(ucode[i]); }
};

