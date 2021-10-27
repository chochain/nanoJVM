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
	struct Klass *next;
	struct Klass *supr;
	FILE         *src;
	union{
		NativeClass *native = 0;
		struct{
			U8 interfaces;
			U8 fields;
			U8 methods;
			U8 rsv;
		};
	};
	U16 instDataSize;
	U16 instDataOfst;
	U16 clsDataSize;
	U16 clsDataOfst;
	U8  data[];
};
struct Klass *g_cls_root;

U8 getBE8(FILE *f, IU addr) {
    U32   offset = (U32)addr;         // file offset
	if ((U32)ftell(f) != offset) {
		if (fseek(f, offset, SEEK_SET)==-1) {
			fprintf(stderr, "ERR %d: fseek(f, %x)\n", errno, offset);
			exit(-1);	
		}
	}
	S8 v;
	if (fread(&v, 1, 1, f)==-1) {
		fprintf(stderr, "ERR: fread()\n");
		exit(-2);	
	}
	printf(" %02x%c", (U8)v, v < 0x20 ? '_' : (char)v);
	return v;
}
S32 getBE32(FILE *f, IU addr) {
	S32 i32 = 0;
	for(U8 i = 0; i < 4; i++) i32 = (i32 << 8) | getBE8(f, addr++);
	return i32;
}
/*
U24 getBE24(U32 h, IU addr) {
	U16 i24 = 0;
	for(U8 t8 = 0; t8 < 3; t8++) i24 = (i24 << 8) | getBE8(h, addr++);
	return i24;
}
*/
S16 getBE16(FILE *f, IU addr) {
	S16 i16 = (S16)getBE8(f, addr++) << 8;
	return i16 | getBE8(f, addr);
}
IU skipAttr(FILE *f, IU addr){
    return addr + 6 + getBE32(f, addr + 2);
}
U8 typeSize(char type){
	switch(type){
    case TYPE_BYTE:   case TYPE_BOOL:  return 1;
    case TYPE_CHAR:   case TYPE_SHORT: return 2;
    case TYPE_DOUBLE: case TYPE_LONG:  return 8;
    default: return 4;
	}
}
U16 poolOffset(FILE *f, U16 idx){
    IU addr = 10;
    while (idx--) {
        U8 t = getBE8(f, addr++);
        printf("\n%d:%x", t, addr);
        switch(t){
        case CONST_STRING:  addr += 2 + getBE16(f, addr); break;
        case CONST_STR_REF:
        case CONST_CLASS:   addr += 2; break;
        case CONST_LONG:
        case CONST_DOUBLE:  addr += 8; idx--;  break;
        default:            addr += 4; break;
        }
    }
    return addr;
}
#define MAGIC      0xcafebabe
#define ERR_MAGIC  1
#define ERR_SUPER  2
#define ERR_MEMORY 3
int load_class(FILE *f, struct Klass **pcls) {
	if ((U32)getBE32(f, 0) != MAGIC) return ERR_MAGIC;

    U16 n_cnst = getBE16(f, 8) - 1;			       // number of constant pool entries
    IU addr = poolOffset(f, n_cnst);               // skip constant descriptors
    U16 n_intf = getBE16(f, addr + 6);             // number of interfaces
    U16 p_intf = (addr += 8);                      // pointer to interface section
    U16 p_attr = (addr += (n_intf << 1));	       // pointer to attribute section
    U16 n_attr = getBE16(f, addr); addr += 2;      // fetch number of attributes
    
    U16 clsDatSz = 0, instDatSz = 0;
    while (n_attr--) {                             // scan attributes
        U16 flag = getBE16(f, addr);		       // get flags
        bool isClassVar = !!(flag & ACC_STATIC);
			
        U16 idx  = getBE16(f, addr + 4);	        // read type destriptor index
        U8  type = getBE8(f, poolOffset(f, idx) + 3); // get type descriptor first character
        U8  sz   = typeSize(type);
        if (isClassVar) clsDatSz  += sz;
        else            instDatSz += sz;		
			
        U16 asz = getBE16(f, addr + 6);            // get number of attributes
        addr += 8;
        while (asz--) addr = skipAttr(f, addr);
    }
    U16 p_fld = p_intf + (n_intf << 1) + 2;
    addr += 2;	//now points to methods

    struct Klass *supr = 0;
    U16 cidx = getBE16(f, p_intf - 4);			    // super class index
    if (cidx) {
        cidx = getBE16(f, poolOffset(f, cidx) + 1);	// super class name index
        supr = (struct Klass*)poolOffset(f, cidx);  // offset to super class
    }
	//now we have enough data to know this class's size -> alloc it
    U16   csz  = sizeof(Klass) + clsDatSz + (supr ? supr->clsDataOfst + supr->clsDataSize : 0);
	struct Klass *cls = (Klass*)malloc(csz);
	if (!cls) return ERR_MEMORY;
	
	cls->src  = f;
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
    
	*pcls = g_cls_root = cls;
	return 0;
}

int main(int ac, char* av[]) {
	if(ac < 1){
		fprintf(stderr, "Usage:> %s f.class\n", av[0]);
		return -1;
	}
    FILE* f = fopen(av[1], "rb");
    if (!f) {
        fprintf(stderr," Failed to open file\n");
        return -1;
    }
    struct Klass *cls;
    if (load_class(f, &cls)) return 1;
#if 0    
	ret = ujInitAllClasses();
	if(ret != UJ_ERR_NONE){
		fprintf(stderr, "ujInitAllClasses() fail\n");
		return -1;	
	}
	//now classes are loaded, time to call the entry point
	
	threadH = ujThreadCreate(0);
	if(!threadH){
		fprintf(stderr, "ujThreadCreate() fail\n");
		return -1;	
	}
	i = ujThreadGoto(threadH, mainClass, "main", "()V");
	while (ujCanRun()) {
		i = ujInstr();
		if(i != UJ_ERR_NONE){
			fprintf(stderr, "Ret %d @ instr right before 0x%08lX\n", i, ujThreadDbgGetPc(threadH));
			exit(-10);
		}
	}
#endif
    return 0;
}
