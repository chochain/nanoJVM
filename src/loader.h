#ifndef NANOJVM_LOADER_H
#define NANOJVM_LOADER_H
#include "common.h"
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
class ClassFile {
#if ARDUINO
    File f;
#else
    FILE *f;
#endif
	const char *fname;
    U8   type_size(char type);
    U16  attr_size(U16 addr);
    U8   field_size(U16 &addr);

    void create_method(char *cls, U16 &m_root, U16 &addr);
    
public:
	IU   ctx;             /// context (class addr in dictionary)

    ClassFile(const char *fname);
    IU   load(IU jdx);

    U8   getU8(U16 addr);
    U16  getU16(U16 addr);
    U32  getU32(U16 addr);
    U16  offset(U16 idx, bool debug=false);

    char *getStr(U16 addr, char *buf, bool ref=false);
};
///
/// Class File Manager
///
extern ClassFile *clsfile[CLSFILE_MAX];
class Loader {
	static int cnt;
public:
	static int active()            { return cnt ? cnt - 1 : 0; }
	static ClassFile *get(int jcf) { return clsfile[jcf]; }
	static int load(const char *fname);
};
#endif // NANOJVM_LOADER_H
