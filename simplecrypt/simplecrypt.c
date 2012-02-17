/* Simplecrypt - Simple Gmail file obfuscator. Public domain.
 * Use for executable attachments or archives containing executables.
 * Written in 2012 by Kirn Gill II <segin2005@gmail.com>
 * Idea by Mattis Michel <sic_zer0@hotmail.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	FILE *fd, *fd2;
	char a;
	if(argc < 2) {
		fprintf(stderr, "Not enough arguments!\n");
		exit(1);
	}
	if(!(fd = fopen(argv[1], "rb")) || !(fd2 = fopen(argv[2], "wb"))) {
		perror(argv[fd ? 2 : 1]);
		exit(1);
	}
	while(fread(&a, 1, 1, fd)) {
		a ^= 0xFF;
		fwrite(&a, 1, 1, fd2);
	}
	fclose(fd);
	fclose(fd2);
	exit(0);
}
