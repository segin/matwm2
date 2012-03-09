/* stdlib/atexit.c: atexit() registration and _atexitproc() completion
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#define INTERNAL 

#include <stdlib.h>

void *_atexitregs[ATEXIT_MAX];
int _atexitcount;

int atexit(void (*function)(void))
{
	if(_atexitcount == ATEXIT_MAX) return -1;
	_atexitregs[_atexitcount - 1] = function;
	_atexitcount++;
}

void _atexitproc(void)
{
	int i;
	void (*function)(void);
	for(i = _atexitcount; i > 0; i--) {
		function = _atexitregs[i - 1];
		(*function)();
	}
}
