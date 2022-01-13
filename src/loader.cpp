#include "loader.h"
#include "mmu.h"
///
/// Loader - private methods
///
U8 ClassFile::type_size(char type){
    switch(type){
    case TYPE_BYTE:   case TYPE_BOOL:  return 1;
    case TYPE_CHAR:   case TYPE_SHORT: return 2;
    case TYPE_DOUBLE: case TYPE_LONG:  return 8;
    default: return 4;
    }
}
IU ClassFile::attr_size(IU addr){
    return (IU)6 + getU32(addr + 2);
}
U8 ClassFile::field_size(IU &addr) {
    U16 ifld = getU16(addr + 2);                 // field name index
    U16 itype= getU16(addr + 4);                 // read type destriptor index
    U16 xsz  = getU16(addr + 6);                 // get number of filed attributes
    U8  type = getU8(offset(itype-1) + 3);       // get type descriptor first character
    addr += 8;                                   // pointer to field attributes
    while (xsz--) addr += attr_size(addr);      //

    return type_size(type);
}
void ClassFile::create_method(char *cls, IU &m_root, IU &addr) {
    U16 i_name  = getU16(addr + 2);
    U16 i_parm  = getU16(addr + 4);
    U16 n_attr  = getU16(addr + 6);
    addr += 8;

    IU  mjdx = addr + 14;
    U32 len  = getU32(mjdx - 4);

    char name[128], parm[32];
    getStr(i_name, name);
    getStr(i_parm, parm);

#if ENABLE_DEBUG
    LOG("\n  ["); LOX2(i_name); LOG("]");
    LOG(cls);  LOG("::");
    LOG(name); LOG(parm); LOG(" ("); LOX(len); LOG(" bytes)");
#endif // ENABLE_DEBUG

    IU pidx = gPool.get_parm_idx(parm);
    gPool.add_method(m_root, name, mjdx, pidx);

    while (n_attr--) addr += attr_size(addr);
}
///
/// Loader - public methods
///
U8 ClassFile::getU8(IU addr) {
    FSEEK(f, addr);
    return FGETC(f);
}
U16 ClassFile::getU16(IU addr) {
    U16 v = (U16)getU8(addr) << 8;
    return v | FGETC(f);
}
U32 ClassFile::getU32(IU addr) {
    U32 v = (U32)getU8(addr);
    for(U8 i = 0; i < 3; i++) v = (v << 8) | FGETC(f);
    return v;
}
char *ClassFile::getStr(IU idx, char *buf, bool ref) {
    IU n = offset(idx - 1);    	    /// [17]:00b6:1=>ej32/Forth, or
    if (ref) {                      /// [12]008e:7=>17
    	n = getU16(n + 1);     		/// 17
    	n = offset(n - 1);          /// [17]:00b6:1=>ej32/Forth
    }
    U16 i, len = getU16(n + 1);
    FSEEK(f, n + 3);		        /// move cursor to string
    for (i=0; i<len; i++)   {
        buf[i] = FGETC(f);
    }
    buf[i] = '\0';
    return buf;
}
///
/// calculate offset of an index in constant pool
///
U16 ClassFile::offset(U16 jdx, bool debug) {
    IU addr = 10;
    ///
    /// loop over the constant pool to get the offset
    /// TODO: this is a very inefficient way of keeping indices
    ///
    for (int i=0; i<jdx; i++) {
        U8 t = getU8(addr);
        if (debug) {
            LOG("\n["); LOX2(i+1); LOG("]"); LOX4(addr); LOG(":"); LOX2(t);
        }
        addr++;
        switch(t){
        case CONST_INT:
        	if (debug) { LOG("=>0x"); LOX(getU32(addr)); }
            addr += 4; break;
        case CONST_UTF8:
            if (debug) {
                LOG("=>");
                for (U16 i=0, n=getU16(addr); i<n; i++) {
                    CHR(getU8(addr+2+i));
                }
            }
            addr += 2 + getU16(addr); break;
        case CONST_STRING:
        case CONST_CLASS:
        	if (debug) { LOG("=>"); LOX(getU16(addr)); }
            addr += 2; break;
        case CONST_LONG:
        case CONST_DOUBLE:
            addr += 8; i++;  break;
        case CONST_FIELD:
        case CONST_METHOD:
        case CONST_NAME_TYPE:
            if (debug) {
                LOG("=>["); LOX(getU16(addr));
                LOG(",");   LOX(getU16(addr+2)); LOG("]");
            }
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
ClassFile::ClassFile(const char *fname) {
	this->fname = fname;
#if ARDUINO
    if (!SPIFFS.begin()) { LOG("failed to open SPIFFS"); }
    f = SPIFFS.open(fname, "r");
    if (!f.available()) { LOG("failed to open file: "); LOG(fname); }
#else
    f = fopen(fname, "rb");
    if (!f) { LOG("failed to open file: "); LOG(fname); }
#endif
#if ENABLE_DEBUG
    LOG("\nJava class file: "); LOG(fname);
#endif // ENABLE_DEBUG
#if LOADER_DUMP
    ///
    /// dump Java class file content
    ///
    U32 sz = FSIZE(f);
    FSEEK(f, 0);
	char buf[17] = { 0 };
    for (IU i=0; i<=sz; i+=16) {
        LOG("\n"); LOX4(i); LOG(": ");
        for (int j=0; j<16; j++) {
            char c = FGETC(f);
            buf[j] = ((c>0x7f)||(c<0x20)) ? '_' : c;
            LOX2((int)c); LOG(j%4==3 ? "  " : " ");
        }
        buf[16] = '\0';
        LOG(buf);
    }
#endif // LOADER_DUMP
}
U16 ClassFile::load() {
    if ((U32)getU32(0) != MAGIC) return ERR_MAGIC;

    U16 n_cnst = getU16(8) - 1;                 // number of constant pool entries
    IU  addr   = offset(n_cnst, LOADER_DUMP);   // skip constant descriptors
    U16 acc    = getU16(addr);  addr += 2;      // class access flag
    U16 i_cls  = getU16(addr);  addr += 2;      // this class
    U16 i_supr = getU16(addr);  addr += 2;      // super class

    char cls[128], supr[128];
    getStr(i_cls, cls, true);                   // fetch class name
    getStr(i_supr, supr, true);                 // fetch super class name

    U16 n_intf = getU16(addr);  addr += 2;      // number of interfaces
    IU  p_intf = addr;                          // pointer to interface section
    U16 n_fld  = getU16((addr += n_intf*2));    // number of fields
    IU  p_fld  = (addr += 2);

    U16 sz_cv = 0, sz_iv = 0;
    while (n_fld--) {                           // scan fields
        U16 flag = getU16(addr);                // get access flags
        bool is_cls = flag & ACC_STATIC;
        U8 sz = field_size(addr);               // process one field_info
        if (is_cls) sz_cv += sz;
        else        sz_iv += sz;
    }
    U16 n_method = getU16(addr);                // number of methods
    IU  p_method = (addr += 2);                 // pointer to methods

#if LOADER_DUMP
    LOG("\nclass ["); LOX(i_cls);  LOG("]"); LOG(cls);
    LOG(" : [");      LOX(i_supr); LOG("]"); LOG(supr);
    LOG("\n  p_intf=");   LOX(p_intf);   LOG(", p_attr=");   LOX(p_fld);
    LOG("\n  sz_cls=");   LOX(sz_cv);    LOG(", sz_inst=");  LOX(sz_iv);
    LOG("\n  n_method="); LOX(n_method); LOG(", p_method="); LOX(p_method);
    LOG("\n} loaded.");
#endif // LOADER_DUMP
    
    IU  m_root = DATA_NA;
    for (int i=0; i<n_method; i++) {
    	create_method(cls, m_root, addr);
    }
    return this->cls_id = gPool.add_class(cls, m_root, supr, sz_cv, sz_iv);
}
///
/// Loader class implementation
///
ClassFile *clsfile[CLSFILE_MAX];
int Loader::cnt = 0;
int Loader::load(const char *fname) {
	ClassFile *cf = new ClassFile(fname);
	if (cf) {
		cf->load();
		clsfile[cnt++] = cf;
	}
	return cnt;
}
