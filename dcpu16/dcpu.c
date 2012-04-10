/* Emulator for the DCPU-16, as featured in the game 0x10c
 * ISA specification is copyright 2012, Mojang Ab. Based on ISA spec v1.1
 * See 0x10c.com for details / updated ISA specifications.
 * This emulator is copyright 2012 Kirn Gill <segin2005@gmail.com>
 * See COPYING for license text.
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#define bt(l,x,u) (x >= l && x <= u)
#define opnd operand[o ? 0 : 1]
#define opndc tmps[o ? 0 : 1]

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
	XOP_JSR = 1 /* jump to subroutine */ 
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

enum desttype { 
	D_REG = 0x10000,
	D_MEM = 0x20000,
	D_STK = 0x30000,
	D_ERR = 0xF0000 /* short immediates, too! */
};

int main(int argc, char *argv[]) 
{
	/* the registers */ 
	uint16_t regs[11];

	/* main system memory */
	uint16_t mem[0x10000];
	
	/* CPU internals */
	
	uint16_t op_code, operand[2], op_tmp, o;
	uint32_t tmps[2];

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
		op_tmp = (op_code >> o ? 4 : 10) & 0x3f;
		/* decode meaning of operand */
		if(bt(0,op_tmp,7)) 
			opndc = D_REG + op_tmp;
		else if(bt(8,op_tmp,0xf)) {
			opndc = mem regs[op_tmp-8];
		} else if(bt(0x10,op_tmp,0x17)) {

			opndc = D_MEM + ((++regs[REG_PC]+regs[op_tmp-0x10]) & 0xffff);
		} else if(bt(0x20,op_tmp,0x3f))
			opnd = op_tmp - 0x20;
		else switch (op_tmp) { 
		case 0x18: /* POP */
			opnd = regs[REG_SP]++;
			cycles++;
			break;
		case 0x19; /* PEEK */
			opnd = regs[REG_SP];
			cycles++;
			break;
		case 0x1a: /* PUSH */
			opnd = --reg[REG_SP];
			cycles++;
			break;
		
		}
		
		
		
		switch(o) {
		case 0: 
			o = 1; 
			goto decode_operand;
		case 2:
			goto non_basic_exec:
		};
		
		
fetch_operand: 
		op_tmp = (op_code >> o ? 4 : 10) & 0x3f;
		/* switch statement was too wordy
		 * also here we actually fetch for operand
		 */
		if(bt(0,op_tmp,7)) 
			opnd = reg[op_tmp];
		else if(bt(8,op_tmp,0xf)) {
			opnd = mem[regs[op_tmp-8]];
			cycles++;
		} else if(bt(0x10,op_tmp,0x17)) {
			/*wat do if this overflow?*/
			opnd = mem[++regs[REG_PC]+regs[op_tmp-0x10]];
			cycles++;
		} else if(bt(0x20,op_tmp,0x3f))
			opnd = op_tmp - 0x20;
		else switch (op_tmp) { 
		case 0x18: /* POP */
			opnd = regs[REG_SP]++;
			cycles++;
			break;
		case 0x19; /* PEEK */
			opnd = regs[REG_SP];
			cycles++;
			break;
		case 0x1a: /* PUSH */
			opnd = --reg[REG_SP];
			cycles++;
			break;
		
		}



basic_exec:
		
		
		
	continue;
non_basic:
	o = 2;
	goto decode_operand;
non_basic_exec:
		switch (op_a) { 
		case XOP_JSR:
			/* do shit */
			break;
		default:
			/* illegal */ 
			regs[REG_PC]++;
		}
	}
}
