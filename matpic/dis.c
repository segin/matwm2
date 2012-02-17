#include "host.h" /* NULL */
#include "dis.h" /* dsym_t */
#include "misc.h" /* fawarn() */
#include "main.h" /* infile, address */

arr_t dsym = { NULL, 0, 0, 0 };

void dwarn(char *msg) {
	fawarn(infile, address, msg);
}

int disassemble(char **ret) {

}

