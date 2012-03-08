/* stdlib/exit.c: implement exit() 
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#include <stdlib.h>

void exit(int status)
{
	_atexitproc();
	_exit(status & 0377);
}