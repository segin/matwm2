/***********************
 * assembler interface *
 ***********************/

#include <stdlib.h> /* free(), NULL */
#include "ppc.h"
#include "as.h"
#include "dis.h"
#include "ihex.h"
#include "misc.h" /* errexit(), cleanup(), readfile() */
#include "io.h"

int main(int argc, char *argv[]) {
	char *a, *code;
	int i;
	/* teh command line options */
	int disasm = 0, ppm = 0, through = 0;

	mstdio_init();
	clearfile();
	for (i = 0; i < argc; ++i) {
		if (*(argv[i]) == '-') {
			a = argv[i] + 1;
			argv[i] = NULL;
			while (*a) {
				switch (*a) {
					case 'd':
						++disasm;
						break;
					case 'p':
						ppm = 1; /* only preprocess */
						break;
					case 'P':
						ppm = 2; /* don't preprocess */
						break;
					case 't': /* assemble then disassemble */
						++through;
						break;
					default:
						errexit("invalid argument");
				}
				++a;
			}
		}
	}

	code = readfile(NULL);
	if (code == NULL)
		errexit("failed to read file '%s'", file);
	if (!disasm) {
		if (ppm == 1) { /* only preprocess */
			preprocess(mstdout, code);
			free(code);
			goto done;
		} if (ppm == 0) { /* normal preprocess */
			ioh_t *memout = mmemopen(0);
			preprocess(memout, code);
			free(code);
			mfwrite(memout, "\0", 1);
			code = mmemget(memout);
			mfclose(memout);
		}
		if (code == NULL)
			errexit("error error");
		assemble(code);
		free(code); /* release the monster */
		if (through) {
			ioh_t *memout = mmemopen(0);
			getihex(memout);
			mfwrite(memout, "\0", 1);
			code = mmemget(memout);
			mfclose(memout);
			if (code == NULL)
				errexit("error error");
		} else getihex(mstdout);
	}
	if (disasm || through) {
		readihex(code);
		free(code);
		disassemble(mstdout);
	}
	done:
	cleanup();

	return EXIT_SUCCESS;
}
