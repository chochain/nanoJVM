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
			IU p_intf;
			IU p_fld;
			IU p_method;
		};
	};
	U16 sz_cls;
	U16 off_cls;
	U16 sz_inst;
	U16 off_inst;
	U8  data[];
};
struct Klass *g_cls_root;

U8 getU8(FILE *f, IU addr) {
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
	return v;
}
U16 getU16(FILE *f, IU addr) {
	U16 v = (U16)getU8(f, addr++) << 8;
	return v | getU8(f, addr);
}
U32 getU32(FILE *f, IU addr) {
	U32 v = 0;
	for(U8 i = 0; i < 4; i++) v = (v << 8) | getU8(f, addr++);
	return v;
}
IU skipAttr(FILE *f, IU addr){
    return addr + 6 + getU32(f, addr + 2);
}
U8 typeSize(char type){
	switch(type){
    case TYPE_BYTE:   case TYPE_BOOL:  return 1;
    case TYPE_CHAR:   case TYPE_SHORT: return 2;
    case TYPE_DOUBLE: case TYPE_LONG:  return 8;
    default: return 4;
	}
}
void dump(FILE *f, IU a0, IU sz) {
    for (IU i=a0; i<=(a0+sz); i+=16) {
        printf("\n%04x: ", i);
        for (int j=0; j<16; j++) {
            U8 c = getU8(f, i+j);
            printf("%02x%s", (U16)c, (j%4==3) ? "  " : " ");
        }
        for (int j=0; j<16; j++) {   // print and advance to next byte
            U8 c = getU8(f, i+j);
            printf("%c", (char)((c==0x7f||c<0x20) ? '_' : c));
        }
    }
}
U16 poolOffset(FILE *f, U16 idx){
    IU addr = 10;
    while (idx--) {
        U8 t = getU8(f, addr++);
        switch(t){
        case CONST_STRING: 	addr += 2 + getU16(f, addr); break;
        case CONST_STR_REF:
        case CONST_CLASS:   addr += 2; break;
        case CONST_LONG:
        case CONST_DOUBLE:  addr += 8; idx--;  break;
        default:            addr += 4; break;
        }
    }
    return addr;
}
void printStr(FILE *f, IU addr) {
	U16 len = getU16(f, addr);
	printf("[%x]=>", addr);
	for (U16 i=0; i<len; i++) {
		printf("%c", (char)getU8(f, addr+2+i));
	}
}
void getConstName(FILE *f, U16 cidx) {
    printf("\nidx=%x", cidx);
    if (cidx) {
    	cidx = poolOffset(f, cidx-1);				// offset to index (1-based)
        printf("\noff=%x", cidx);                   // bytecode:1
        cidx = getU16(f, cidx+1);				    // str const index (bytecode:1)
        printf("\nstridx=%x", cidx);
        cidx = poolOffset(f, cidx-1);				// offset to name str (1-based)
        printf("\nname");
        printStr(f, cidx+1);
    }
}
U8 getInfo(FILE *f, IU &addr) {
    U16 ifld = getU16(f, addr + 2);                 // field name index
    U16 itype= getU16(f, addr + 4);	                // read type destriptor index
    U16 xsz  = getU16(f, addr + 6);                 // get number of filed attributes
    addr += 8;                                      // pointer to field attributes
    while (xsz--) addr = skipAttr(f, addr);         //

    U8  type = getU8(f, poolOffset(f, itype-1) + 3);  // get type descriptor first character
    U8  sz   = typeSize(type);
    return sz;
}
#if 0
ClassFile {        // Java class file format
    u4             magic;
    u2             minor_version;
    u2             major_version;
    u2             constant_pool_count;
    cp_info        constant_pool[constant_pool_count-1];
    u2             access_flags;
    u2             this_class;
    u2             super_class;
    u2             interfaces_count;
    u2             interfaces[interfaces_count];
    u2             fields_count;
    field_info     fields[fields_count];
    u2             methods_count;
    method_info    methods[methods_count];
    u2             attributes_count;
    attribute_info attributes[attributes_count];
}
cp_info {
    u1 tag;
    u1 info[];
}
field_info {
    u2             access_flags;
    u2             name_index;
    u2             descriptor_index;
    u2             attributes_count;
    attribute_info attributes[attributes_count];
}
method_info {
    u2             access_flags;
    u2             name_index;
    u2             descriptor_index;
    u2             attributes_count;
    attribute_info attributes[attributes_count];
}
attribute_info {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 info[attribute_length];
}
#endif
#define MAGIC      0xcafebabe
#define ERR_MAGIC  1
#define ERR_SUPER  2
#define ERR_MEMORY 3
int load_class(FILE *f, struct Klass **pcls) {
	dump(f, 0,400);
	if ((U32)getU32(f, 0) != MAGIC) return ERR_MAGIC;

    U16 n_cnst = getU16(f, 8) - 1;			        // number of constant pool entries
    IU addr = poolOffset(f, n_cnst);                // skip constant descriptors
    U16 acc    = getU16(f, addr);					// class access flag
    U16 i_cls  = getU16(f, addr + 2);				// this class
    U16 i_supr = getU16(f, addr + 4);				// super class
    U16 n_intf = getU16(f, addr + 6);               // number of interfaces
    IU  p_intf = (addr += 8);                       // pointer to interface section
    U16 n_fld  = getU16(f, (addr += n_intf*2));     // number of fields
    IU  p_fld  = (addr += 2);
    printf("\np_intf=%x, np_attr=%x", p_intf, p_fld);
    
    U16 sz_cls = 0, sz_inst = 0;
    while (n_fld--) {                               // scan fields
        U16 flag = getU16(f, addr);		            // get access flags
        bool is_cls = flag & ACC_STATIC;
    	U8 sz = getInfo(f, addr);                   // process one field_info
        if (is_cls) sz_cls  += sz;
        else        sz_inst += sz;
    }
    printf("\nsz_cls=%x, sz_inst=%x", sz_cls, sz_inst);

    U16 n_method = getU16(f, addr);                 // number of methods
    IU  p_method = (addr += 2);						// pointer to methods
    printf("\nn_method=%x, p_method=%x", n_method, p_method);
    while (n_method--) {
        U16 flag = getU16(f, addr);		            // get access flags
        bool is_cls = flag & ACC_STATIC;
    	U8 sz = getInfo(f, addr);
    }

    getConstName(f, i_cls);
    getConstName(f, i_supr);

    struct Klass *supr = 0;
    return 0;
	//now we have enough data to know this class's size -> alloc it
    U16   csz  = sizeof(Klass) + sz_cls + (supr ? supr->off_cls + supr->sz_cls : 0);
	struct Klass *cls = (Klass*)malloc(csz);
	if (!cls) return ERR_MEMORY;
	
	cls->src  = f;
	cls->supr = supr;
	if (supr) {
		cls->off_cls  = supr->off_cls  + supr->sz_cls;
		cls->off_inst = supr->off_inst + supr->sz_inst;
	}
	else{
		cls->off_cls  = 0;
		cls->off_inst = 0;
	}
	cls->p_intf    = p_intf;
	cls->p_fld     = p_fld;
	cls->p_method  = p_method;
	cls->sz_cls    = sz_cls;
	cls->sz_inst   = sz_inst;
	cls->next      = g_cls_root;
    
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
