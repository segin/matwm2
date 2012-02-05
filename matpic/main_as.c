/***********************
 * assembler interface *
 ***********************/

#include <stdlib.h> /* exit(), EXIT_FAILURE */
#include <stdio.h>

#include "as.h"

main() {
	char *code = " org 0x200 \ntest pest\r\n vest\nrest   \n data 1, 2, 3, test\n addwf 2, 1";

	assemble(&code);

	{
		int i;
		ins_t *ins;
		for (i = 0; i < inss.count; ++i) {
			ins = (ins_t *) ((ins_t *) inss.data) + i;
			printf("line %d, type %d:\n", ins->line, ins->type);
			switch (ins->type) {
				case IT_ORG:
					printf(" org 0x%X\n", ins->address);
					break;
				case IT_DAT:
					printf(" data 0x%X\n", ins->value);
					break;
				case IT_INS:
					printf(" opcode 0x%X\n", ins->oc);
			}
		}
	}

	cleanup();

	return EXIT_SUCCESS;
}
