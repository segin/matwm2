/***********************
 * assembler interface *
 ***********************/

#include <stdlib.h> /* free(), NULL */
#include "ppc.h"
#include "as.h"
#include "dis.h"
#include "ihex.h"
#include "misc.h" /* errexit(), cleanup(), readfile(), file, infile */
#include "io.h"

int main(int argc, char *argv[]) {
	char *a, *code;
	int i;
	ioh_t *out = out;
	/* teh command line options */
	int disasm = 0, ppm = 0, through = 0;

	mstdio_init();
	out = mstdout;
	for (i = 1; i < argc; ++i) {
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
		} else {
			if (file != NULL) {
				if (out != mstdout)
					errexit("too many arguments");
				out = mfopen(argv[i], MFM_WR | MFM_CREAT | MFM_TRUNC);
				if (out == NULL)
					errexit("cannot open output file '%s'", argv[i]);
			} else {
				file = infile = argv[i]; /* only setting file to check against it later */
			}
		}
	}

	code = readfile(file);
	if (code == NULL)
		errexit("failed to read file '%s'", infile);
	if (!disasm) {
		if (ppm == 1) { /* only preprocess */
			preprocess(out, code);
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
		} else getihex(out);
	}
	if (disasm || through) {
		readihex(code);
		free(code);
		disassemble(out);
	}
	done:
	cleanup();

	return EXIT_SUCCESS;
}
