/* Simplecrypt - Simple Gmail file obfuscator. Public domain.
 * Use for executable attachments or archives containing executables.
 * Written in 2012 by Kirn Gill II <segin2005@gmail.com>
 */

#include <stdio.h>

int main(int argc, char *argv[]) {
	FILE *fd, fd2;
	if(!(fd = fopen(argv[1], "rb") || !(fd2 = fopen(argv[2], "wb") {
		if(fd)
			perror(argv[2]);
		else
			perror(argv[1]);
		exit;
	}
	char a;
	while(fread(&a, 1, 1, fd)) {
		a ^= 0xFF;
		fwrite(&a, 1, 1, fd2);
	}
	fclose(fd);
	fclose(fd2);
}
