/* stdlib/exit.c: implement exit() 
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#define INTERNAL 
 
#include <stdlib.h>
#include <unistd.h>

void exit(int status)
{
	_atexitproc();
	_exit(status & 0377);
}