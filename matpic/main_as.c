/***********************
 * assembler interface *
 ***********************/

#include "host.h" /* EXIT_SUCCESS, errexit(), FILE, stdin, stdout, fread(),
                     realloc(), free(), NULL */
#include "as.h"
#include "misc.h" /* errexit() */

int main(int argc, char *argv[]) {
	char *code;

	FILE *infd = stdin;
	FILE *outfd = stdout;

	while (!feof(infd)) {
		
	}

	assemble(code);

	{
		int i;
		ins_t *ins;
		for (i = 0; i < inss.count; ++i) {
			ins = (ins_t *) ((ins_t *) inss.data) + i;
			switch (ins->type) {
				case IT_ORG:
					printf(" org 0x%X\n", ins->org.address);
					break;
				case IT_DAT:
					printf(" data 0x%X\n", ins->data.value);
					break;
				case IT_INS:
					printf(" opcode 0x%X\n", ins->ins.oc);
					break;
			}
		}
	}

	cleanup();

	return EXIT_SUCCESS;
}

