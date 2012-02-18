/***********************
 * assembler interface *
 ***********************/

#include "host.h" /* EXIT_SUCCESS, errexit(), FILE, stdin, stdout, fread(),
                     realloc(), free(), NULL */
#include "as.h"
#include "dis.h"
#include "misc.h" /* errexit() */
#include "mem.h" /* BLOCK */
#include "ihex.h"

char *infile = "<stdin>";
unsigned int address = 0, line = 1;

void reset(void) {
	line = 1;
	infile = "<file>";
	address = 0;
	arr_free(&inss);
	arr_free(&labels);
	/* reset disassembler too */
	arr_free(&dsym);
}

int main(int argc, char *argv[]) {
	FILE *infd = stdin;
	FILE *outfd = stdout;
	char *code = NULL;

	{ /* read teh filez */
		int pos = 0, mem = 0;

		while (!feof(infd)) {
			if (ferror(infd))
				errexit("failed to read file");
			if (pos == mem) {
				if (mem + BLOCK < mem)
					errexit("wtf integer overflow");
				code = (char *) realloc((void *) code, mem + BLOCK);
				mem += BLOCK;
				if (code == NULL)
					errexit("out of memory");
			}
			pos += fread((void *) code, 1, mem - pos, infd);
		}
		code[pos] = 0;
	}
	assemble(code);
	free(code); /* release the monster */

	{ /* writes output */
		int len = getihex(&code);
		readihex(code);
		disassemble(&code);
		fwrite((void *) code, 1, len, outfd);
	}

	cleanup();

	return EXIT_SUCCESS;
}

