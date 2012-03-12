
#include <unistd.h>

void _start(char *args, ...)
{
	write(1, "Hello, World!\n", 14);
	_Exit(0);
}

