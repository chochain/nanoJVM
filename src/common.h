///
/// @brief nanoJVM common types and configuration
///
#ifndef NANOJVM_COMMON_H
#define NANOJVM_COMMON_H
#include <stdint.h>     // int16_t, ...
#include <stdlib.h>     // strtol
#include <string.h>     // strcmp
using namespace std;
///
/// conditional compilation options
///
#define RANGE_CHECK     1
#define ENABLE_DEBUG    1
///
/// memory block size setting
///
#define PMEM_SZ         1024*16     /** parameter space            */
#define HEAP_SZ         1024*16     /** object space               */
#define RS_SZ           128         /** return stack size per VM   */
#define SS_SZ           256         /** data stack size per thread */
#define CONST_SZ        128         /** constant pool size         */
#define OP_LU_SZ        4			/** Forth opcode lookup table  */
#define VT_LU_SZ        64          /** Java method lookup table   */
#define CV_LU_SZ        16          /** max class variables        */
#define IV_LU_SZ        16          /** max instance variables     */
#define DATA_NA         0xffff      /** memory pool negate index   */
///
/// Arduino support macros
///
#if ARDUINO
#include <Arduino.h>
#include "SPIFFS.h"
#include "FS.h"
#else
#include <stdio.h>
#include <chrono>
#include <thread>
#define millis()        chrono::duration_cast<chrono::milliseconds>( \
                            chrono::steady_clock::now().time_since_epoch()).count()
#define delay(ms)       this_thread::sleep_for(chrono::milliseconds(ms))
#define yield()         this_thread::yield()
#define PROGMEM
#endif // ARDUINO
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
/// console and file/SPIFFS IO macros
///
#if ARDUINO
#define LOG(s)      Serial.print(s)
#define CHR(c)      Serial.print((char)(c))
#define LOX(h)      Serial.print(h, HEX)
#define LOX2(h)     LOX((h)>>4); LOX((h)&0xf)
#define LOX4(h)     LOX2((h)>>8); LOX2((h)&0xff)
#define FSIZE(f)    f.size()
#define FSEEK(f, o) f.seek(o, SeekSet)
#define FGETC(f)    ((U8)f.read())
#else
#define LOG(s)      printf("%s", s)
#define CHR(c)      printf("%c", (char)c)
#define LOX(h)      printf("%x", h)
#define LOX2(h)     printf("%02x", (U8)(h))
#define LOX4(h)     printf("%04x", h)
#define FSIZE(f)    (fseek(f, 0L, SEEK_END), ftell(f))
#define FSEEK(f, o) fseek(f, o, SEEK_SET)
#define FGETC(f)    ((U8)fgetc(f))
#endif // ARDUINO
#endif // NANOJVM_COMMON_H
