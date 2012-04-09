/* Emulator for the DCPU-16, as featured in the game 0x10c
 * ISA specification is copyright 2012, Mojang Ab. Based on ISA spec v1.1
 * See 0x10c.com for details / updated ISA specifications.
 * This emulator is copyright 2012 Kirn Gill <segin2005@gmail.com>
 * See COPYING for license text.
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

enum basicops { 
	OP_NONBASIC = 0, /* extended opcode */
	OP_SET, /* sorta like MOV in most archs */
	OP_ADD, /* addition */
	OP_SUB, /* subtraction */
	OP_MUL, /* multiplicaton */
	OP_DIV, /* division */
	OP_MOD, /* modulus */
	OP_SHL, /* barrel-shift left */
	OP_SHR, /* barrel-shift right */
	OP_AND, /* binary and */
	OP_BOR, /* BORing operator, I kid, binary or */
	OP_XOR, /* exclusive or, used by shellcode to make zeros */
	/* The following are WTF instructions */
	OP_IFE, /* execute following op if operands are equal */
	OP_IFN, /* execute following op if operands aren't equal */
	OP_IFG, /* execute following op if first operand greater than second */
	/* and finally, the most WTF of the basic instructions */
	OP_IFB  /* execute following op if binary and of operands is not zero */
};

enum nonbasicops { 
	XOP_JSR = 1 /* jump and save return */ 
};

enum operands {
	IMM_A = 0, /* register immediates */
	IMM_B,
	IMM_C,
	IMM_X,
	IMM_Y, 
	IMM_Z,
	IMM_I,
	IMM_J,
	PTR_A, /* register is a pointer */
	PTR_B,
	PTR_C,
	PTR_X,
	PTR_Y,
	PTR_Z,
	NWP_A, /* pointer is (PC + 1) + register ? */
	NWP_B,
	NWP_C,
	NWP_X,
	NWP_Y,
	NWP_Z,
	NWP_I,
	NWP_J,
	/* Can these be used opposite of their normal intended use?*/
	POP,   /* read at stack pointer, increment stack pointer */
	PEEK,  /* read (write?) at stack pointer */
	PUSH,  /* write to stack pointer, decrement stack pointer */
	IMM_SP, /* stack pointer as a register immediate */
	IMM_PC, /* program counter as a register immediate */
	IMM_O, /* Overflow indicator as a register immediate */
	NWP,   /* pointer at PC + 1 */
	IMP   /* literal at PC + 1 */
	/* Everything from here to 0x3f is an immediate literal - 0x20 */
};

enum registers { 
	REG_A = 0,
	REG_B,
	REG_C,
	REG_X,
	REG_Y,
	REG_Z,
	REG_I,
	REG_J,
	REG_PC,
	REG_O,
	REG_SP
};



int main(int argc, char *argv[]) 
{
	/* the registers */ 
	uint16_t regs[11];

	/* main system memory */
	uint16_t mem[0x10000];
	
	/* CPU internals */
	
	uint16_t op_code, op_a, op_b, op_tmp, o;
	uint32_t tmp_ax, tmp_bx;

	/* And finally, a cycle counter for benchmarking */
	uint64_t cycles;

	/* fuck you, goto is perfectly fine if used correctly */
init:
	for(o = REG_A ; o <= REG_O ; o++) reg[o] = 0;
	reg[REG_SP] = 0xffff;

	/* CPU main loop */
	while(1) { 
		op_code = mem[regs[REG_PC]];
		o = 0;
		
		if(!(op_code & 0xf)) goto non_basic;

decode_operand: 		
		switch((op_code >> o ? 4 : 10) & 0x3f) { 
			




		switch(o) {
		case 0: 
			o = 1; 
			goto decode_operand;
		case 2:
			goto non_basic_exec:
		};

basic_exec:
		
		
		
non_basic:
	o = 2;
	goto decode_operand;
non_basic_exec:
		switch (op_a) { 
			case XOP_JSR:
			/* do shit */
			break;
		}
	}
}
