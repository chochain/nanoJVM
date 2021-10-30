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
#define CONST_UTF8		1
#define CONST_INT		3
#define CONST_FLOAT		4
#define CONST_LONG		5
#define CONST_DOUBLE	6
#define CONST_CLASS		7
#define CONST_STRING	8
#define CONST_FIELD		9
#define CONST_METHOD	10
#define CONST_INTERFACE	11
#define CONST_NAME_TYPE	12
#define CONST_MHNDL     15
#define CONST_MTYPE     16
#define CONST_INVOKE    18
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
	IU  lfa;		/// index to previous class
	IU  supr;		/// index to super class
	IU  intf;       /// index to interface
	IU  pfa;		/// index to parameter fields
	IU  vt;			/// index to method table
	U16 cvsz;		/// size of class variables
	U16 ivsz;		/// size of instance variables
	U16 len;        /// class name string length
	U8  data[];		/// raw data (string name)
};
IU   g_cls_root = 0;
FILE *f;

U8 getU8(IU addr) {
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
U16 getU16(IU addr) {
	U16 v = (U16)getU8(addr++) << 8;
	return v | getU8(addr);
}
U32 getU32(IU addr) {
	U32 v = 0;
	for(U8 i = 0; i < 4; i++) v = (v << 8) | getU8(addr++);
	return v;
}
U8 typeSize(char type){
	switch(type){
    case TYPE_BYTE:   case TYPE_BOOL:  return 1;
    case TYPE_CHAR:   case TYPE_SHORT: return 2;
    case TYPE_DOUBLE: case TYPE_LONG:  return 8;
    default: return 4;
	}
}
void dump(IU a0, IU sz) {
    for (IU i=a0; i<=(a0+sz); i+=16) {
        printf("\n%04x: ", i);
        for (int j=0; j<16; j++) {
            U8 c = getU8(i+j);
            printf("%02x%s", (U16)c, (j%4==3) ? "  " : " ");
        }
        for (int j=0; j<16; j++) {   // print and advance to next byte
            U8 c = getU8(i+j);
            printf("%c", (char)((c==0x7f||c<0x20) ? '_' : c));
        }
    }
}
void printStr(IU addr, const char *hdr=0) {
	U16 len = getU16(addr);
	printf("%s", hdr ? hdr : "=>");
	for (U16 i=0; i<len; i++) {
		printf("%c", (char)getU8(addr+2+i));
	}
}
U16 poolOffset(U16 idx, bool debug=false) {
    IU addr = 10;
    for (int i=0; i<idx; i++) {
        U8 t = getU8(addr);
        if (debug) printf("\n[%02x]%04x:%x", i+1, addr, t);		// Constant Pool is 1-based
        addr++;
        switch(t){
        case CONST_UTF8:
        	if (debug) printStr(addr);
        	addr += 2 + getU16(addr); break;
        case CONST_STRING:
        case CONST_CLASS:
        	if (debug) printf("=>%x", getU16(addr));
        	addr += 2; break;
        case CONST_LONG:
        case CONST_DOUBLE:
        	addr += 8; i++;  break;
        case CONST_FIELD:
        case CONST_METHOD:
        case CONST_NAME_TYPE:
        	if (debug) printf("=>[%x,%x]", getU16(addr), getU16(addr+2));
        	addr += 4; break;
        default: addr += 4; break;
        }
    }
    return addr;
}
void getConstName(U16 cidx, bool ref=false) {
    printf("\nname[%02x]", cidx);
    if (cidx) {
    	cidx = poolOffset(cidx-1);				// offset to index (1-based)
        printf("%04x:", cidx);                 	// bytecode:1
        if (ref) {
            cidx = getU16(cidx+1);				// str const index (bytecode:1)
        	printf("[%x]", cidx);
        	cidx = poolOffset(cidx-1);			// offset to name str (1-based)
        }
        printStr(cidx+1);
    }
}
IU skipAttr(IU addr){
    return addr + 6 + getU32(addr + 2);
}
U8 getInfo(IU &addr) {
    U16 ifld = getU16(addr + 2);                 // field name index
    U16 itype= getU16(addr + 4);	             // read type destriptor index
    U16 xsz  = getU16(addr + 6);                 // get number of filed attributes
    U8  type = getU8(poolOffset(itype-1) + 3);   // get type descriptor first character
    addr += 8;                                   // pointer to field attributes
    while (xsz--) addr = skipAttr(addr);         //

    return typeSize(type);
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
int load_class(struct Klass **pcls) {
	if ((U32)getU32(0) != MAGIC) return ERR_MAGIC;

    U16 n_cnst = getU16(8) - 1;			        // number of constant pool entries
    IU addr = poolOffset(n_cnst, true);         // skip constant descriptors
    U16 acc    = getU16(addr);	addr += 2;		// class access flag
    U16 i_cls  = getU16(addr);	addr += 2;		// this class
    U16 i_supr = getU16(addr);	addr += 2;		// super class

    getConstName(i_cls, true);
    getConstName(i_supr, true);
    Klass *supr = 0;	/* search super class */

    U16 n_intf = getU16(addr);	addr += 2;      // number of interfaces
    IU  p_intf = addr;                       		// pointer to interface section
    U16 n_fld  = getU16((addr += n_intf*2));     // number of fields
    IU  p_fld  = (addr += 2);
    printf("\np_intf=%x, np_attr=%x", p_intf, p_fld);
    
    U16 sz_cv = 0, sz_iv = 0;
    while (n_fld--) {                               // scan fields
        U16 flag = getU16(addr);		            // get access flags
        bool is_cls = flag & ACC_STATIC;
    	U8 sz = getInfo(addr);                      // process one field_info
        if (is_cls) sz_cv += sz;
        else        sz_iv += sz;
    }
    printf("\nsz_cls=%x, sz_inst=%x", sz_cv, sz_iv);

    U16 n_method = getU16(addr);                    // number of methods
    IU  p_method = (addr += 2);						// pointer to methods
    printf("\nn_method=%x, p_method=%x", n_method, p_method);
    while (n_method--) {
        U16 flag = getU16(addr);		            // get access flags
        bool is_cls = flag & ACC_STATIC;
    	U8 sz = getInfo(addr);
    }

	//now we have enough data to know this class's size -> alloc it
    U16 sz = sizeof(Klass) + sz_cv + (supr ? supr->cvsz : 0);
	struct Klass *cls = (Klass*)malloc(sz);
	if (!cls) return ERR_MEMORY;

	cls->lfa   = g_cls_root;
	cls->supr  = i_supr;
	cls->intf  = p_intf;
	cls->pfa   = p_fld;
	cls->vt    = p_method;
	cls->cvsz  = sz_cv;
	cls->ivsz  = sz_iv;
    
	*pcls = cls;
	g_cls_root = i_cls;		/* reset class root */
	return 0;
}

IU getMethod(Klass *cls, const char *fname, const char *param) {
	IU  addr = cls->vt;
	U16 n_method = getU16(addr - 2);
	U16 m_match  = 0;
	while (n_method--) {
		U16 acc     = getU16(addr);
		U16 i_name  = getU16(addr + 2);
		U16 i_parm  = getU16(addr + 4);
		U16 n_attr  = getU16(addr + 6);
		addr += 8;
		getConstName(i_name);
		getConstName(i_parm);
		while (n_attr--) {
			if (m_match++) {
				U16 i_code = getU16(addr);
				getConstName(i_code);
				bool t_match = true;  			/* attr is a Code */
				if (t_match) return addr;
			}
			addr = skipAttr(addr);
		}
	}
	return 0;
}

#include "ucode.h"
#include "jvm.h"
#include <iostream>
extern Ucode  gUcode;
extern Ucode  gForth;
extern Pool   gPool;
extern Thread t0;
extern void (*fout_cb)(int, const char*);  /// forth output callback function
extern void ss_dump();

void run(Klass *cls, IU addr) {
    static auto send_to_con = [](int len, const char *rst) { cout << rst; };
    gPool.register_class("Ucode", gUcode.vtsz, gUcode.vt);
    gPool.register_class("nanojvm/Forth", gForth.vtsz, gForth.vt, "Ucode");
    fout_cb = send_to_con;                 /// setup callback function

    printf("\n\nnanoJVM starting...\n\n");

    t0.PC = addr + 14;                     /// pointer to class file
	U16 n_local = getU16(addr + 8);
	/* allocate local stack */
	while (t0.PC!=0xffff) {
		U8 op = getU8(t0.PC++);
		printf("%04x:%02x exec", t0.PC-1, op);
		gUcode.exec(t0, op);	           /* execute JVM opcode */
		ss_dump();
	}

    printf("\nnanoJVM done.\n");
}

int main(int ac, char* av[]) {
	if(ac < 1){
		fprintf(stderr, "Usage:> %s f.class\n", av[0]);
		return -1;
	}
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
    f = fopen(av[1], "rb");
    if (!f) {
        fprintf(stderr," Failed to open file\n");
        return -1;
    }
    ///
    /// dump class file
    ///
	fseek(f, 0L, SEEK_END);
	U32 fsz = ftell(f);
	dump(0, fsz);
	///
	/// load class file
	///
    struct Klass *cls;
    if (load_class(&cls)) return 1;
	///
	/// execution Java main program
	///
    IU addr = getMethod(cls, "main", "()V");
    if (addr) run(cls, addr);

    return 0;
}
