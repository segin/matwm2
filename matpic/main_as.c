/***********************
 * assembler interface *
 ***********************/

#include <stdlib.h> /* exit(), EXIT_FAILURE */
#include <stdio.h>

#include "as.h"

main() {
	char *code = " org 0x200 \ntest file \"abc\"\n data\n Data 1, 2, 3, test, $\n data $, 10\n data $\n addwf 2, 1\n data $\n data $";

	assemble(&code);

	{
		int i;
		ins_t *ins;
		for (i = 0; i < inss.count; ++i) {
			ins = (ins_t *) ((ins_t *) inss.data) + i;
			switch (ins->type) {
				case IT_ORG:
					printf(" org 0x%X\n", ins->address);
					break;
				case IT_DAT:
					printf(" data 0x%X\n", ins->value);
					break;
				case IT_INS:
					printf(" opcode 0x%X\n", ins->oc);
					break;
			}
		}
	}

	cleanup();

	return EXIT_SUCCESS;
}
