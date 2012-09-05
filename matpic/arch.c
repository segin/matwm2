#include "arch.h"
#include "pic14b.h"
#include "pic18f.h"

arch_t *arch = &ARCH_DEFAULT;
arch_t *archs[] = { &pic14b, &pic18f, NULL };

void setarch(char *name) {
	arch_t **a = archs;
	while (*a != NULL) {
		if (strcasecmp(name, (*a)->name) == 0) {
			arch = *a;
			return;
		}
		++a;
	}
	errexit("invalid architecture '%s'", name);
}
