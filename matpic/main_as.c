/***********************
 * assembler interface *
 ***********************/

#include "host.h" /* EXIT_SUCCESS, errexit(), FILE, stdin, stdout, fread(),
                     realloc(), free(), NULL */
#include "as.h"
#include "misc.h" /* errexit() */

int main(int argc, char *argv[]) {
	FILE *infd = stdin;
	FILE *outfd = stdout;
	char *code = NULL;

	{ /* read teh fail */
		int pos = 0, mem = 0;

		while (!feof(infd)) {
			if (ferror(infd))
				errexit("failed to read file");
			if (pos == mem) {
				if (mem + BLOCK < mem)
					errexit("wtf integer overflow");
				code = realloc(code, mem + BLOCK);
				mem += BLOCK;
				if (code == NULL)
					errexit("out of memory");
			}
			pos += fread((void *) code, 1, mem - pos, infd);
		}
		code[pos] = 0;
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

