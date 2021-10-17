# JVM instruction set
## Operand stack (operand stack) related

### Local variable table (local variable table) to operand stack (operand stack)
* iload	Push the specified int type local variable to the top of the stack
* iload_x	Push the xth int type local variable to the top of the stack, x range: [0, 3]
* lload	Push the specified long type local variable to the top of the stack
* lload_x	Push the xth long local variable to the top of the stack, x range: [0, 3]
* fload	Push the specified float type local variable to the top of the stack
* fload_x	Push the xth flaot type local variable to the top of the stack, x range: [0, 3]
* dload	Push the specified double type local variable to the top of the stack
* dload_x	Push the xth double type local variable to the top of the stack, x range: [0, 3]
* aload	Push the specified reference type local variable to the top of the stack
* aload_x	Push the x-th reference type local variable to the top of the stack, x range: [0, 3]
 
It can be seen that the instruction operand type at the beginning of i is integer type, the instruction operand type at the beginning of l is long type, the instruction operand type at the beginning of f is float type, the instruction operand type at the beginning of d is double, and the instruction operand type at the beginning of a The instruction operand type is a reference type (reference).

### Operand stack (operand stack) to local variable table (local variable table)
* istore	Store the int value of the top of the stack into the specified local variable
* istore_x	Store the int type value on the top of the stack into the xth local variable, x range: [0, 3]
* lstore	Store the long value on the top of the stack into the specified local variable
* lstore_x	Store the long value on the top of the stack into the xth local variable, x range: [0, 3]
* fstore	Store the float value on the top of the stack into the specified local variable
* fstore_x	Store the float value on the top of the stack into the xth local variable, x range: [0, 3]
* dstore	Store the double value on the top of the stack into the specified local variable
* dstore_x	Store the double value on the top of the stack into the xth local variable, the range of x: [0, 3]
* astore	Store the top reference type of the stack in the specified local variable
* astore_x	Store the top reference type of the stack in the xth local variable, x range: [0, 3]

### Constant to operand stack
* bipush	Push the single-byte constant value (-128~127) to the top of the stack
* sipush	Push a short integer constant value (-32768~32767) to the top of the stack
* ldc	Push int, float or String constant values ​​from the constant pool to the top of the stack
* ldc_w	Push int, float or String constant values ​​from the constant pool to the top of the stack (wide index)
* aconst_null	Push null to the top of the stack
* iconst_m1	Push int -1 to the top of the stack
* iconst_x	Push int type x to the top of the stack, x range: [0, 5]
* lconst_x	Push long type x to the top of the stack, x range: [0, 1]
* fconst_x	Push float type x to the top of the stack, x range: [0, 1]
* dconst_x	Push double type x to the top of the stack, x range: [0, 1]

### Push the array with the specified index of the array to the operand stack (operand stack)
* iaload	Push the value of the specified index of the int type array to the top of the stack
* laload	Push the value of the specified index of the long array to the top of the stack
* faload	Push the value of the specified index of the float type array to the top of the stack
* daload	Push the value of the specified index of the double array to the top of the stack
* aaload	Push the value of the specified index of the reference array to the top of the stack
* baload	Push the value of the specified index of the boolean or byte array to the top of the stack
* caload	Push the value of the specified index of the char array to the top of the stack
* saload	Push the value of the specified index of the short array to the top of the stack

### Store the operand stack number to the specified index of the array
* iastore	Store the top int value of the stack in the specified index position of the specified array
* lastore	Store the long value on the top of the stack into the specified index position of the specified array
* fastore	Store the float value on the top of the stack into the specified index position of the specified array
* dastore	Store the double value on the top of the stack into the specified index position of the specified array
* aastore	Store the top reference value of the stack in the specified index position of the specified array
* bastore	Store the boolean or byte value at the top of the stack into the specified index position of the specified array
* castore	Store the top char value of the stack into the specified index position of the specified array
* sastore	Store the short value on the top of the stack into the specified index position of the specified array

### Operand stack other related
* pop	Pop the top value of the stack (the value cannot be of type long or double)
* pop2	Pop one (long or double type) or two values ​​at the top of the stack (other)
* dup	Copy the top value of the stack and push the copied value onto the top of the stack
* dup_x1	Copy the top value of the stack and push the two copied values ​​onto the top of the stack
* dup_x2	Copy the top value of the stack and push three (or two) copied values ​​onto the top of the stack
* dup2	Copy one (long or double type) or two (other) values ​​from the top of the stack and push the copied value onto the top of the stack
* dup2_x1	Copy the top 2 values ​​of the stack and push them to the top of the stack twice
* dup2_x2	Copy the top 2 values ​​of the stack and push them to the top of the stack 3 times
* swap	Swap the top two values ​​of the stack (the values ​​cannot be of type long or double)

## Calculation related
* plus	iadd,ladd,fadd,dadd
* Less	is,ls,fs,ds
* Multiply	imul,lmul,fmul,dmul
* except	idiv,ldiv,fdiv,ddiv
* remainder	irem,lrem,frem,drem
* Negative	ineg,lneg,fneg,dneg
* Shift	ishl,lshr,iushr,lshl,lshr,lushr
* Bitwise or	ior,lor
* Bitwise and	iand,land
* Bitwise XOR	ixor,lxor

### Type conversion	i2l, i2f, i2d, l2f, l2d, f2d (relaxed numerical conversion); i2b, i2c, i2s, l2i, f2i, f2l, d2i, d2l, d2f (narrowed numerical conversion)
### Conditional transfer
* Conditional transfer	ifeq,iflt,ifle,ifne,ifgt,ifge,ifnull,ifnonnull,if_icmpeq,if_icmpene, if_icmplt,if_icmpgt,if_icmple,if_icmpge,if_acmpeq,if_acmpne,lcmp,fcmpl,fcmpg,dcmpl,dcmpg
* Compound conditional transfer	tableswitch,lookupswitch
* Unconditional transfer	goto,goto_w,jsr,jsr_w,ret
 
### Class and array
* Create class	new
* Create new array	newarray,anewarray,multianwarray
* Access class domain and class instance domain	getfield,putfield,getstatic,putstatic
* Get array length	arraylength
* Phase detection class instance or array attribute	instanceof,checkcast
 
### Other instructions
* A practical way to schedule objects	invokevirtual
* Call the method implemented by the interface	invokeinterface
* Call instance methods that require special handling	invokespecial
* Call static methods in named classes	invokestatic
* Method returns	ireturn,lreturn,freturn,dreturn,areturn,return
* abnormal	athrow
* Implementation of the finally keyword	jsr,jsr_w,ret
* synchronized lock	monitorenter,monitorexit

---
# All instruction set
* Script	Mnemonic	Description
* 0×00	nop	do nothing
* 0×01	aconst_null	Push null to the top of the stack
* 0×02	iconst_m1	Push int -1 to the top of the stack
* 0×03	iconst_0	Push int 0 to the top of the stack
* 0×04	iconst_1	Push the int type 1 to the top of the stack
* 0×05	iconst_2	Push int type 2 to the top of the stack
* 0×06	iconst_3	Push the int type 3 to the top of the stack
* 0×07	iconst_4	Push the int type 4 to the top of the stack
* 0×08	iconst_5	Push the int type 5 to the top of the stack
* 0×09	lconst_0	Push long 0 to the top of the stack
* 0x0a	lconst_1	Push long type 1 to the top of the stack
* 0x0b	fconst_0	Push float 0 to the top of the stack
* 0x0c	fconst_1	Push float type 1 to the top of the stack
* 0x0d	fconst_2	Push float 2 to the top of the stack
* 0x0e	dconst_0	Push double type 0 to the top of the stack
* 0x0f	dconst_1	Push double type 1 to the top of the stack
* 0×10	bipush	Push the single-byte constant value (-128~127) to the top of the stack
* 0×11	sipush	Push a short integer constant value (-32768~32767) to the top of the stack
* 0×12	ldc	Push int, float or String constant values ​​from the constant pool to the top of the stack
* 0×13	ldc_w	Push int, float or String constant values ​​from the constant pool to the top of the stack (wide index)
* 0×14	ldc2_w	Push the long or double constant value from the constant pool to the top of the stack (wide index)
* 0×15	iload	Push the specified int type local variable to the top of the stack
* 0×16	lload	Push the specified long type local variable to the top of the stack
* 0×17	fload	Push the specified float type local variable to the top of the stack
* 0×18	dload	Push the specified double type local variable to the top of the stack
* 0×19	aload	Push the specified reference type local variable to the top of the stack
* 0x1a	iload_0	Push the 0th int local variable to the top of the stack
* 0x1b	iload_1	Push the first int type local variable to the top of the stack
* 0x1c	iload_2	Push the second int type local variable to the top of the stack
* 0x1d	iload_3	Push the third int type local variable to the top of the stack
* 0x1e	lload_0	Push the 0th long local variable to the top of the stack
* 0x1f	lload_1	Push the first long local variable to the top of the stack
* 0×20	lload_2	Push the second long local variable to the top of the stack
* 0×21	lload_3	Push the third long local variable to the top of the stack
* 0×22	fload_0	Push the 0th float type local variable to the top of the stack
* 0×23	fload_1	Push the first float type local variable to the top of the stack
* 0×24	fload_2	Push the second float type local variable to the top of the stack
* 0×25	fload_3	Push the third float type local variable to the top of the stack
* 0×26	dload_0	Push the 0th double local variable to the top of the stack
* 0×27	dload_1	Push the first double type local variable to the top of the stack
* 0×28	dload_2	Push the second double type local variable to the top of the stack
* 0×29	dload_3	Push the third double type local variable to the top of the stack
* 0x2a	aload_0	Push the 0th reference type local variable to the top of the stack
* 0x2b	aload_1	Push the first reference type local variable to the top of the stack
* 0x2c	aload_2	Push the second reference type local variable to the top of the stack
* 0x2d	aload_3	Push the third reference type local variable to the top of the stack
* 0x2e	iaload	Push the value of the specified index of the int type array to the top of the stack
* 0x2f	laload	Push the value of the specified index of the long array to the top of the stack
* 0×30	faload	Push the value of the specified index of the float type array to the top of the stack
* 0×31	daload	Push the value of the specified index of the double array to the top of the stack
* 0×32	aaload	Push the value of the specified index of the reference array to the top of the stack
* 0×33	baload	Push the value of the specified index of the boolean or byte array to the top of the stack
* 0×34	caload	Push the value of the specified index of the char array to the top of the stack
* 0×35	saload	Push the value of the specified index of the short array to the top of the stack
* 0×36	istore	Store the int value of the top of the stack into the specified local variable
* 0×37	lstore	Store the long value on the top of the stack into the specified local variable
* 0×38	fstore	Store the float value on the top of the stack into the specified local variable
* 0×39	dstore	Store the double value on the top of the stack into the specified local variable
* 0x3a	astore	Store the top reference value of the stack into the specified local variable
* 0x3b	istore_0	Store the int value on the top of the stack into the 0th local variable
* 0x3c	istore_1	Store the top int value of the stack into the first local variable
* 0x3d	istore_2	Store the int value on the top of the stack into the second local variable
* 0x3e	istore_3	Store the int value on the top of the stack into the third local variable
* 0x3f	lstore_0	Store the long value on the top of the stack into the 0th local variable
* 0×40	lstore_1	Store the long value on the top of the stack into the first local variable
* 0×41	lstore_2	Store the long value on the top of the stack into the second local variable
* 0×42	lstore_3	Store the long value on the top of the stack into the third local variable
* 0×43	fstore_0	Store the float value on the top of the stack into the 0th local variable
* 0×44	fstore_1	Store the float value on the top of the stack into the first local variable
* 0×45	fstore_2	Store the float value on the top of the stack into the second local variable
* 0×46	fstore_3	Store the float value on the top of the stack into the third local variable
* 0×47	dstore_0	Store the double value on the top of the stack into the 0th local variable
* 0×48	dstore_1	Store the double value on the top of the stack into the first local variable
* 0×49	dstore_2	Store the double value on the top of the stack into the second local variable
* 0x4a	dstore_3	Store the double value on the top of the stack into the third local variable
* 0x4b	astore_0	Store the top reference value of the stack into the 0th local variable
* 0x4c	astore_1	Store the top reference value of the stack into the first local variable
* 0x4d	astore_2	Store the top reference value of the stack into the second local variable
* 0x4e	astore_3	Store the top reference value of the stack into the third local variable
* 0x4f	iastore	Store the top int value of the stack in the specified index position of the specified array
* 0×50	lastore	Store the long value on the top of the stack into the specified index position of the specified array
* 0×51	fastore	Store the float value on the top of the stack into the specified index position of the specified array
* 0×52	dastore	Store the double value on the top of the stack into the specified index position of the specified array
* 0×53	aastore	Store the top reference value of the stack in the specified index position of the specified array
* 0×54	bastore	Store the boolean or byte value at the top of the stack into the specified index position of the specified array
* 0×55	castore	Store the top char value of the stack into the specified index position of the specified array
* 0×56	sastore	Store the short value on the top of the stack into the specified index position of the specified array
* 0×57	pop	Pop the top value of the stack (the value cannot be of type long or double)
* 0×58	pop2	Pop one (long or double type) or two values ​​at the top of the stack (other)
* 0×59	dup	Copy the top value of the stack and push the copied value onto the top of the stack
* 0x5a	dup_x1	Copy the top value of the stack and push the two copied values ​​onto the top of the stack
* 0x5b	dup_x2	Copy the top value of the stack and push three (or two) copied values ​​onto the top of the stack
* 0x5c	dup2	Copy one (long or double type) or two (other) values ​​from the top of the stack and push the copied value onto the top of the stack
* 0x5d	dup2_x1	Copy the top 2 values ​​of the stack and push them to the top of the stack twice
* 0x5e	dup2_x2	Copy the top 2 values ​​of the stack and push them to the top of the stack 3 times
* 0x5f	swap	Swap the top two values ​​of the stack (the values ​​cannot be of type long or double)
* 0×60	iadd	Add the two int values ​​at the top of the stack and push the result onto the top of the stack
* 0×61	ladd	Add the two long values ​​at the top of the stack and push the result onto the top of the stack
* 0×62	fadd	Add the two float values ​​at the top of the stack and push the result onto the top of the stack
* 0×63	dadd	Add the two double values ​​at the top of the stack and push the result onto the top of the stack
* 0×64	isub	Subtract the two int values ​​from the top of the stack and push the result onto the top of the stack
* 0×65	lsub	Subtract the two long values ​​from the top of the stack and push the result onto the top of the stack
* 0×66	fsub	Subtract the two float values ​​from the top of the stack and push the result onto the top of the stack
* 0×67	dsub	Subtract the two double values ​​from the top of the stack and push the result onto the top of the stack
* 0×68	imul	Multiply the top two int values ​​of the stack and push the result onto the top of the stack
* 0×69	lmul	Multiply the two long values ​​at the top of the stack and push the result onto the top of the stack
* 0x6a	fmul	Multiply the top two float values ​​of the stack and push the result onto the top of the stack
* 0x6b	dmul	Multiply the top two double values ​​of the stack and push the result onto the top of the stack
* 0x6c	idiv	Divide the two int values ​​at the top of the stack and push the result onto the top of the stack
* 0x6d	ldiv	Divide the two long values ​​at the top of the stack and push the result onto the top of the stack
* 0x6e	fdiv	Divide the two float values ​​at the top of the stack and push the result onto the top of the stack
* 0x6f	ddiv	Divide the two double values ​​at the top of the stack and push the result onto the top of the stack
* 0×70	irem	Perform modulo operation on the top two int values ​​of the stack and push the result onto the top of the stack
* 0×71	lrem	Perform modulo operation on the top two long values ​​of the stack and push the result onto the top of the stack
* 0×72	frem	Perform modulo operation on the two float values ​​on the top of the stack and push the result onto the top of the stack
* 0×73	drem	Perform modulo operation on the top two double values ​​of the stack and push the result onto the top of the stack
* 0×74	ineg	Take the negative value of the int type on the top of the stack and push the result onto the top of the stack
* 0×75	lneg	Take the long value at the top of the stack negative and push the result onto the top of the stack
* 0×76	fneg	Take the negative value of the float type on the top of the stack and push the result onto the top of the stack
* 0×77	dneg	Take the negative value of the double type on the top of the stack and push the result onto the top of the stack
* 0×78	ishl	Shift the int value to the left by the specified number of bits and push the result onto the top of the stack
* 0×79	lshl	Shift the long value to the left by the specified number of bits and push the result to the top of the stack
* 0x7a	ishr	Shift the int type value to the right (sign) by the specified number of bits and push the result to the top of the stack
* 0x7b	lshr	Shift the long value right (sign) by the specified number of bits and push the result onto the top of the stack
* 0x7c	iushr	Shift the int value right (unsigned) by the specified number of bits and push the result to the top of the stack
* 0x7d	lushr	Shift the long value right (unsigned) by the specified number of bits and push the result to the top of the stack
* 0x7e	iand	Put the two int values ​​on the top of the stack as "bitwise AND" and push the result onto the top of the stack
* 0x7f	land	Take the two long values ​​at the top of the stack as "bitwise AND" and push the result onto the top of the stack
* 0×80	ior	Take the two int values ​​at the top of the stack as "bitwise OR" and push the result onto the top of the stack
* 0×81	lor	Take the two long values ​​at the top of the stack as "bitwise OR" and push the result onto the top of the stack
* 0×82	ixor	Put the two int values ​​on the top of the stack as "bitwise exclusive OR" and push the result onto the top of the stack
* 0×83	lxor	Make the "bitwise exclusive OR" of the two long values ​​at the top of the stack and push the result onto the top of the stack
* 0×84	iinc	Increase the specified value of the specified int type variable. There can be two variables, which respectively represent index and const. Index refers to the index-th int type local variable, and const increases the value
* 0×85	i2l	Coerce the top int value of the stack to a long value and push the result onto the top of the stack
* 0×86	i2f	Coerce the int type value on the top of the stack to a float type value and push the result onto the top of the stack
* 0×87	i2d	Coerce the int type value at the top of the stack to a double type value and push the result onto the top of the stack
* 0×88	l2i	Coerce the long value on the top of the stack to an int value and push the result onto the top of the stack
* 0×89	l2f	Force the long value on the top of the stack into a float value and push the result onto the top of the stack
* 0x8a	l2d	Coerce the long value on the top of the stack to a double value and push the result onto the top of the stack
* 0x8b	f2i	Coerce the float value on the top of the stack to an int value and push the result onto the top of the stack
* 0x8c	f2l	Coerce the top float value of the stack to a long value and push the result onto the top of the stack
* 0x8d	f2d	Coerce the top float value of the stack to a double value and push the result onto the top of the stack
* 0x8e	d2i	Coerce the top double value of the stack to an int value and push the result onto the top of the stack
* 0x8f	d2l	Coerce the top double value of the stack to a long value and push the result onto the top of the stack
* 0×90	d2f	Coerce the top double value of the stack to a float value and push the result onto the top of the stack
* 0×91	i2b	Force the int type value on the top of the stack to a byte type value and push the result onto the top of the stack
* 0×92	i2c	Force the int type value on the top of the stack to a char type value and push the result onto the top of the stack
* 0×93	i2s	Coerce the top int value of the stack to a short value and push the result onto the top of the stack
* 0×94	lcmp	Compare the size of the two long values ​​on the top of the stack, and push the result (1, 0, -1) onto the top of the stack
* 0×95	fcmpl	Compare the size of the two float values ​​on the top of the stack, and push the result (1, 0, -1) to the top of the stack; when one of the values ​​is NaN, push -1 to the top of the stack
* 0×96	fcmpg	Compare the size of the two float values ​​on the top of the stack, and push the result (1, 0, -1) onto the top of the stack; when one of the values ​​is NaN, push 1 onto the top of the stack
* 0×97	dcmpl	Compare the size of the two double values ​​on the top of the stack, and push the result (1, 0, -1) onto the top of the stack; when one of the values ​​is NaN, push -1 onto the top of the stack
* 0×98	dcmpg	Compare the size of the two double values ​​on the top of the stack, and push the result (1, 0, -1) onto the top of the stack; when one of the values ​​is NaN, push 1 onto the top of the stack
* 0×99	ifeq	Jump when the int value on the top of the stack is equal to 0
* 0x9a	ifne	Jump when the int value on the top of the stack is not equal to 0
* 0x9b	iflt	Jump when the int type value on the top of the stack is less than 0
* 0x9c	ifge	Jump when the int value on the top of the stack is greater than or equal to 0
* 0x9d	ifgt	Jump when the int value on the top of the stack is greater than 0
* 0x9e	ifle	Jump when the int value on the top of the stack is less than or equal to 0
* 0x9f	if_icmpeq	Compare the size of the two int types at the top of the stack, and jump when the result is equal to 0
* 0xa0	if_icmpne	Compare the size of the two int types at the top of the stack, and jump when the result is not equal to 0
* 0xa1	if_icmplt	Compare the size of the two int types at the top of the stack, and jump when the result is less than 0
* 0xa2	if_icmpge	Compare the size of the two int types at the top of the stack, and jump when the result is greater than or equal to 0
* 0xa3	if_icmpgt	Compare the size of the two int types at the top of the stack, and jump when the result is greater than 0
* 0xa4	if_icmple	Compare the size of the two int types at the top of the stack, and jump when the result is less than or equal to 0
* 0xa5	if_acmpeq	Compare the two reference values ​​on the top of the stack, and jump when the results are equal
* 0xa6	if_acmpne	Compare the two reference values ​​on the top of the stack, and jump when the results are not equal
* 0xa7	goto	Unconditional jump
* 0xa8	jsr	Jump to the designated 16-bit offset location, and push the address of the next instruction of jsr onto the top of the stack
* 0xa9	ret	Return to the position of the index specified by the local variable (usually combined with jsr, jsr_w)
* 0xaa	tableswitch	Used for switch conditional jump, case value is continuous (variable length instruction)
* 0xab	lookupswitch	Used for switch conditional jump, case value is not continuous (variable length instruction)
* 0xac	ireturn	Return int from current method
* 0xad	lreturn	Return long from current method
* 0xae	freturn	Return float from the current method
* 0xaf	dreturn	Return double from current method
* 0xb0	areturn	Return an object reference from the current method
* 0xb1	return	Return void from the current method
* 0xb2	getstatic	Get the static field of the specified class and push its value to the top of the stack
* 0xb3	putstatic	Assign a value to the static domain of the specified class
* 0xb4	getfield	Get the instance domain of the specified class and push its value onto the top of the stack
* 0xb5	putfield	Assign a value to the instance domain of the specified class
* 0xb6	invokevirtual	Call instance method
* 0xb7	invokespecial	Call the super class constructor, instance initialization method, private method
* 0xb8	invokestatic	Call static method
* 0xb9	invokeinterface	Call interface method
* 0xba	-	
* 0xbb	new	Create an object and push its reference value onto the top of the stack
* 0xbc	newarray	Create an array of the specified primitive type (such as int, float, char...) and push its reference value onto the top of the stack
* 0xbd	anewarray	Create an array of reference type (such as class, interface, array), and push its reference value onto the top of the stack
* 0xbe	arraylength	Get the length value of the array and push it to the top of the stack
* 0xbf	athrow	Throw the exception at the top of the stack
* 0xc0	checkcast	Test type conversion, ClassCastException will be thrown if the test fails
* 0xc1	instanceof	Check whether the object is an instance of the specified class, if it is to push 1 to the top of the stack, otherwise push 0 to the top of the stack
* 0xc2	monitorenter	Acquire the lock of the object, used for synchronization method or synchronization block
* 0xc3	monitorexit	Release the lock of the object, used to synchronize methods or synchronized blocks
* 0xc4	wide	When the index of the local variable exceeds 255, use this instruction to expand the index width.
* 0xc5	multianewarray	create a new array of dimensions dimensions with elements of type identified by class reference in constant pool index (indexbyte1 << 8 + indexbyte2); the sizes of each dimension is identified by count1, [count2, etc.]
* 0xc6	ifnull	if value is null, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 + branchbyte2)
* 0xc7	ifnonnull	if value is not null, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 + branchbyte2)
* 0xc8	goto_w	goes to another instruction at branchoffset (signed int constructed from unsigned bytes branchbyte1 << 24 + branchbyte2 << 16 + branchbyte3 << 8 + branchbyte4)
* 0xc9	jsr_w	jump to subroutine at branchoffset (signed int constructed from unsigned bytes branchbyte1 << 24 + branchbyte2 << 16 + branchbyte3 << 8 + branchbyte4) and place the return address on the stack
* 0xca	breakpoint	reserved for breakpoints in Java debuggers; should not appear in any class file
* 0xcb-0xfd	unnamed	these values are currently unassigned for opcodes and are reserved for future use
* 0xfe	impdep1	reserved for implementation-dependent operations within debuggers; should not appear in any class file
* 0xff	impdep2	reserved for implementation-dependent operations within debuggers; should not appear in any class file

# summary
## JVM instructions are mainly divided into:
* local variable table to operand stack instructions,
* operand stack to local variable table instructions,
* constants to operand stack instructions,
* push the array specified index of the array to the operand stack instruction, and
* operate The number stack is stored in the array designated index instruction,
* operand stack other related instructions,
* operation related instructions,
* conditional transfer instructions,
* class and array instructions and other instructions.
 
The instruction operand type at the beginning of i is integer type, the instruction operand type at the beginning of l is long type, the instruction operand type at the beginning of f is float type, the instruction operand type at the beginning of d is double, and the instruction operand type at the beginning of a Is the reference type (reference).
Load instructions load data from the local variable table to the operand stack, and store instructions store data from the operand stack to the local variable table. The other instructions are mainly used for the operand stack.

