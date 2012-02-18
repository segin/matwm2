#include "host.h" /* exit(), EXIT_FAILURE, fprintf(), stderr */
#include "as.h"
#include "dis.h"
#include "main.h"

void cleanup(void) {
	arr_free(&inss);
	arr_free(&labels);
	arr_free(&dsym);
}

void reset(void) {
	line = 1;
	infile = "<file>";
	address = 0;
	arr_free(&inss);
	arr_free(&labels);
	/* reset disassembler too */
	arr_free(&dsym);
}

void errexit(char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void flerrexit(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void flwarn(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void fawarn(char *file, int addr, char *msg) {
	fprintf(stderr, "%s: %X: %s\n", file, addr, msg);
}

