#include <stdio.h>
#include <memory.h>
#include "core.h"
///
/// class/method ACL flags
///
#define ACC_PUBLIC    	0x0001 	/** may be accessed from outside its package.    */
#define ACC_PRIVATE 	0x0002 	/** accessible only within the defining class.   */
#define ACC_PROTECTED 	0x0004 	/** may be accessed within subclasses.           */
#define ACC_STATIC 	    0x0008 	/** static. isClassVar=true                      */
#define ACC_FINAL 		0x0010 	/** final; may not be overridden.                */
#define ACC_SYNC 	    0x0020 	/** synchronized; invoke wrapped a monitor lock. */
#define ACC_NATIVE 	    0x0100 	/** native; C code                               */
#define ACC_INTERFACE	0x0200	/** Is an interface, not a class.                */
#define ACC_ABSTRACT 	0x0400 	/** abstract; no implementation is provided.     */
#define ACC_STRICT 	    0x0800	/** strict floating-point mode                   */
///
/// types of constants
///
#define CONST_STRING	1
#define CONST_INT		3
#define CONST_FLOAT		4
#define CONST_LONG		5
#define CONST_DOUBLE	6
#define CONST_CLASS		7
#define CONST_STR_REF	8
#define CONST_FIELD		9
#define CONST_METHOD	10
#define CONST_INTERFACE	11
#define CONST_NAMETYPE	12
///
/// types for string-based descriptors
///
#define TYPE_BYTE		'B'
#define TYPE_CHAR		'C'
#define TYPE_DOUBLE	    'D'
#define TYPE_FLOAT		'F'
#define TYPE_INT		'I'
#define TYPE_LONG		'J'
#define TYPE_SHORT		'S'
#define TYPE_BOOL		'Z'
#define TYPE_ARRAY		'['
#define TYPE_OBJ		'L'
#define TYPE_OBJ_END	';'

struct NativeClass {
	const  char* name;
	U16    clsDatSz;
	U16    instDatSz;
	U16    numMethods;
	Method methods[];
};

struct Klass {
	struct Klass* next;
	struct Klass* supr;
	union{
		NativeClass* native = 0;
		struct{
			U8 h;
			U8 interfaces;
			U8 fields;
			U8 methods;
		};
	};
	U16 instDataSize;
	U16 instDataOfst;
	U16 clsDataSize;
	U16 clsDataOfst;
	U8  data[];
};
struct Klass *g_cls_root;

S8 getBE8(U32 h, IU addr) {
    return 0;
}
S32 getBE32(U32 h, IU addr) {
	S32 i32 = 0;
	for(U8 t8 = 0; t8 < 4; t8++) i32 = (i32 << 8) | getBE8(h, addr++);
	return i32;
}
/*
U24 getBE24(U32 h, IU addr) {
	U16 i24 = 0;
	for(U8 t8 = 0; t8 < 3; t8++) i24 = (i24 << 8) | getBE8(h, addr++);
	return i24;
}
*/
S16 getBE16(U32 h, IU addr) {
	S16 i16 = (S16)getBE8(h, addr++) << 8;
	return i16 | getBE8(h, addr);
}
IU skipAttr(U32 h, IU addr){
    return addr + 6 + getBE32(h, addr + 2);
}
U8 typeSize(char type){
	switch(type){
    case TYPE_BYTE:   case TYPE_BOOL:  return 1;
    case TYPE_CHAR:   case TYPE_SHORT: return 2;
    case TYPE_DOUBLE: case TYPE_LONG:  return 8;
    default: return 4;
	}
}
U16 findClass(U32 h, U16 idx){
    IU addr = 10;
    while (--idx) {
        U8 type = getBE8(h, addr++);
        switch(type){
        case CONST_STRING:  addr += 2 + getBE16(h, addr); break;
        case CONST_STR_REF: addr += 2; break;
        case CONST_LONG:
        case CONST_DOUBLE:  addr += 8; idx--;  break;
        case CONST_CLASS:
        default:            addr += 4; break;
        }
    }
    return addr;
}
#define MAGIC      0xcafebabe
#define ERR_MAGIC  1
#define ERR_SUPER  2
#define ERR_MEMORY 3
U8 load_class(U32 h, struct Klass** clsP) {
	if ((U32)getBE32(h, 0) != MAGIC) return ERR_MAGIC;

    IU addr = 10;                       // class file starting address
    U16 n_cls   = getBE16(h, 8) - 1;	// # of constant pool entries
    while (n_cls--) {				    // skip the constants
        U8 type = getBE8(h, addr++);
        switch(type) {
        case CONST_STRING:  addr += 2 + getBE16(h, addr); break;
        case CONST_CLASS:
        case CONST_STR_REF: addr += 2; break;
        case CONST_LONG:
        case CONST_DOUBLE:  addr += 8; n_cls--; break;
        default:            addr += 4; break;
        }
    }
    U16 n_intf = getBE16(h, addr + 6);             // number of interfaces
    addr += 8;
    U16 p_intf = addr;
    addr += n_intf << 1;	                       // skip interfaces
    U16 n_fld  = getBE16(h, addr);                 // fetch number of fields
    addr += 2;
    
    U16 clsDatSz = 0, instDatSz = 0;
    while (n_fld--) {                              // skip fields
        U16 f = getBE16(h, addr);		           // get flags
        bool isClassVar = !!(f & ACC_STATIC);
			
        U16 idx = getBE16(h, addr + 4);	           // read type destriptor index
        U8 type = getBE8(h, findClass(h, idx) + 3);// get type descriptor first character
        U8 sz = typeSize(type);
        if (isClassVar) clsDatSz  += sz;
        else            instDatSz += sz;		
			
        U16 asz = getBE16(h, addr + 6);            // get number of attributes
        addr += 8;
        while (asz--) addr = skipAttr(h, addr);
    }
    addr += 2;	//now points to methods

    struct Klass *supr = 0;
    U16 cidx = getBE16(h, n_intf - 4);			    // super class index
    if (cidx) {
        cidx = getBE16(h, findClass(h, cidx) + 1);	// super class name index
        supr =(struct Klass*)1;
    }
    U16 p_fld = p_intf + (n_intf << 1) + 2;
	if (supr) {	                                    // Object has a superclass?
//		supr = findClass(h, cidx);
		if (!supr) return ERR_SUPER;
	}
    
	//now we have enough data to know this class's size -> alloc it
    U16   csz  = sizeof(Klass) + clsDatSz + (supr ? supr->clsDataOfst + supr->clsDataSize : 0);
	struct Klass *cls = (Klass*)malloc(csz);
	if (!cls) return ERR_MEMORY;
	
	cls->h    = h;
	cls->supr = supr;
	if (supr) {
		cls->instDataOfst = supr->instDataOfst + supr->instDataSize;
		cls->clsDataOfst  = supr->clsDataOfst  + supr->clsDataSize;
	}
	else{
		cls->instDataOfst = 0;
		cls->clsDataOfst  = 0;
	}
	cls->interfaces   = p_intf;
	cls->fields       = p_fld;
	cls->methods      = addr;
	cls->clsDataSize  = clsDatSz;
	cls->instDataSize = instDatSz;
	cls->next         = g_cls_root;
    
	g_cls_root = cls;
	if (clsP) *clsP = cls;

	return 0;
}

int main(int ac, char* av[]) {
}
