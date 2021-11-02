#include "loader.h"

void Loader::init(FILE *cls_file, bool debug) {
    f = cls_file;
    if (!debug) return;
    ///
    /// dump Java class file content
    ///
    fseek(f, 0L, SEEK_END);
    U32 sz = ftell(f);

    rewind(f);
	char buf[17];
    for (IU i=0; i<=sz; i+=16) {
        printf("\n%04x: ", i);
        for (int j=0; j<16; j++) {
            char c = fgetc(f);
            buf[j] = (c==0x7f||c<0x20) ? '_' : c;
            printf("%02x%s", (U8)c, (j%4==3) ? "  " : " ");
        }
        buf[16] = '\0';
        printf("%s", buf);
    }
}
U8 Loader::getU8(IU addr) {
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
U16 Loader::getU16(IU addr) {
    U16 v = (U16)getU8(addr++) << 8;
    return v | getU8(addr);
}
U32 Loader::getU32(IU addr) {
    U32 v = 0;
    for(U8 i = 0; i < 4; i++) v = (v << 8) | getU8(addr++);
    return v;
}
char *Loader::getStr(IU idx, char *buf, bool ref) {
    IU n = poolOffset(idx - 1);    	/// [17]:00b6:1=>nanojvm/Forth, or
    if (ref) {                      /// [12]008e:7=>17
    	n = getU16(n + 1);     		/// 17
    	n = poolOffset(n - 1);      /// [17]:00b6:1=>nanojvm/Forth
    }
    U16 i;
    for (i=0; i<getU16(n + 1); i++)   {
        buf[i] = getU8(n + 3 + i);
    }
    buf[i] = '\0';
    return buf;
}
U8 Loader::typeSize(char type){
    switch(type){
    case TYPE_BYTE:   case TYPE_BOOL:  return 1;
    case TYPE_CHAR:   case TYPE_SHORT: return 2;
    case TYPE_DOUBLE: case TYPE_LONG:  return 8;
    default: return 4;
    }
}
IU Loader::skipAttr(IU addr){
    return addr + 6 + getU32(addr + 2);
}
U8 Loader::getSize(IU &addr) {
    U16 ifld = getU16(addr + 2);                 // field name index
    U16 itype= getU16(addr + 4);                 // read type destriptor index
    U16 xsz  = getU16(addr + 6);                 // get number of filed attributes
    U8  type = getU8(poolOffset(itype-1) + 3);   // get type descriptor first character
    addr += 8;                                   // pointer to field attributes
    while (xsz--) addr = skipAttr(addr);         //

    return typeSize(type);
}
U16 Loader::poolOffset(U16 idx, bool debug) {
    IU addr = 10;
    ///
    /// loop over the constant pool to get the offset
    /// TODO: this is a very inefficient way of keeping indices
    ///
    for (int i=0; i<idx; i++) {
        U8 t = getU8(addr);
        if (debug) printf("\n[%02x]%04x:%x", i+1, addr, t);     // Constant Pool is 1-based
        addr++;
        switch(t){
        case CONST_INT:
            if (debug) printf("=>0x%x", getU32(addr));
            addr += 4; break;
        case CONST_UTF8:
            if (debug) {
                printf("=>");
                for (U16 i=0, n=getU16(addr); i<n; i++) {
                    printf("%c", (char)getU8(addr+2+i));
                }
            }
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
/*
 * JVM class file format references
 *
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
*/
#include "jvm.h"
struct Loader gLoader;

IU Loader::createMethod(IU &addr, IU &m_root) {
    U16 i_name  = getU16(addr + 2);
    U16 i_parm  = getU16(addr + 4);
    U16 n_attr  = getU16(addr + 6);
    addr += 8;

    IU  midx = addr + 14;
    U32 len = getU32(midx - 4);
    char name[128];
    char parm[16];
    printf("\n  [%02x]%s", i_name, getStr(i_name, name));
    printf("%s=%x bytes", getStr(i_parm, parm), len);
    fop op4 = (fop)((P32)getU32(midx));
    Method m = { name, op4, FLAG_DEF };
    gPool.add_method(m, m_root);
    for (int i=4; i<len; i++) {
    	gPool.add_u8(getU8(midx+i));
	}

    while (n_attr--) addr = skipAttr(addr);
}
int Loader::load_class() {
    if ((U32)getU32(0) != MAGIC) return ERR_MAGIC;

    U16 n_cnst = getU16(8) - 1;                 // number of constant pool entries
    IU  addr   = poolOffset(n_cnst, true);      // skip constant descriptors
    U16 acc    = getU16(addr);  addr += 2;      // class access flag
    U16 i_cls  = getU16(addr);  addr += 2;      // this class
    U16 i_supr = getU16(addr);  addr += 2;      // super class

    char cls[128], supr[128];
    printf("\nclass [%x]%s", i_cls, getStr(i_cls,  cls, true));
    printf(" : [%x]%s {", i_supr, getStr(i_supr, supr, true));

    U16 n_intf = getU16(addr);  addr += 2;      // number of interfaces
    IU  p_intf = addr;                          // pointer to interface section
    U16 n_fld  = getU16((addr += n_intf*2));    // number of fields
    IU  p_fld  = (addr += 2);
    printf("\n  p_intf=%x;  p_attr=%x;", p_intf, p_fld);
    
    U16 sz_cv = 0, sz_iv = 0;
    while (n_fld--) {                           // scan fields
        U16 flag = getU16(addr);                // get access flags
        bool is_cls = flag & ACC_STATIC;
        U8 sz = getSize(addr);                  // process one field_info
        if (is_cls) sz_cv += sz;
        else        sz_iv += sz;
    }
    printf("\n  sz_cls=%x;   sz_inst=%x;", sz_cv, sz_iv);

    U16 n_method = getU16(addr);                // number of methods
    IU  p_method = (addr += 2);                 // pointer to methods
    printf("\n  n_method=%x; p_method=%x;", n_method, p_method);
    IU  m_root = 0;
    while (n_method--) {
    	createMethod(addr, m_root);
    }
    printf("\n}\n");
    gPool.add_class(cls, supr, m_root, sz_cv, sz_iv);

    return 0;
}
