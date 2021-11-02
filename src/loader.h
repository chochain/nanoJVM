#ifndef NANOJVM_LOADER_H
#define NANOJVM_LOADER_H
#include <stdio.h>      // fopen and IO
#include <memory.h>     // malloc
#include "core.h"
///
/// types of constants
///
#define CONST_UTF8      1
#define CONST_INT       3
#define CONST_FLOAT     4
#define CONST_LONG      5
#define CONST_DOUBLE    6
#define CONST_CLASS     7
#define CONST_STRING    8
#define CONST_FIELD     9
#define CONST_METHOD    10
#define CONST_INTERFACE 11
#define CONST_NAME_TYPE 12
#define CONST_MHNDL     15
#define CONST_MTYPE     16
#define CONST_INVOKE    18
///
/// types for string-based descriptors
///
#define TYPE_BYTE       'B'
#define TYPE_CHAR       'C'
#define TYPE_DOUBLE     'D'
#define TYPE_FLOAT      'F'
#define TYPE_INT        'I'
#define TYPE_LONG       'J'
#define TYPE_SHORT      'S'
#define TYPE_BOOL       'Z'
#define TYPE_ARRAY      '['
#define TYPE_OBJ        'L'
#define TYPE_OBJ_END    ';'
///
/// signature and error codes
///
#define MAGIC      0xcafebabe
#define ERR_MAGIC  1
#define ERR_SUPER  2
#define ERR_MEMORY 3
///
/// class/method ACL flags
///
#define ACC_PUBLIC      0x0001  /** may be accessed from outside its package.    */
#define ACC_PRIVATE     0x0002  /** accessible only within the defining class.   */
#define ACC_PROTECTED   0x0004  /** may be accessed within subclasses.           */
#define ACC_STATIC      0x0008  /** static. isClassVar=true                      */
#define ACC_FINAL       0x0010  /** final; may not be overridden.                */
#define ACC_SYNC        0x0020  /** synchronized; invoke wrapped a monitor lock. */
#define ACC_NATIVE      0x0100  /** native; C code                               */
#define ACC_INTERFACE   0x0200  /** Is an interface, not a class.                */
#define ACC_ABSTRACT    0x0400  /** abstract; no implementation is provided.     */
#define ACC_STRICT      0x0800  /** strict floating-point mode                   */
///
/// Java class file loader
///
struct Loader {
    FILE *f = 0;

    void init(FILE *cls_file, bool debug=true);

    U8   getU8(IU addr);
    U16  getU16(IU addr);
    U32  getU32(IU addr);
    char *getStr(IU addr, char *buf, bool ref=false);
    
    U8   typeSize(char type);
    IU   skipAttr(IU addr);
    U8   getSize(IU &addr);

    U16  poolOffset(U16 idx, bool debug=false);
    IU   getMethod(const char *fname, const char *param);
    IU   createMethod(IU &addr, IU &m_root);
    
    int  load_class();
    int  run(Thread &t);
};
extern Loader gLoader;
#endif // NANOJVM_LOADER_H
