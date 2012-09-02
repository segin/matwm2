#include <stdio.h>

#define chipsize (4096 *2) /* word is two bytes */

unsigned char data[chipsize];

int main(int argc, char argv[]) {
	unsigned int i, c, len, addr, type, addr_old = 0;
	unsigned int sum = 0;
	/* make map of chip memory */
	for (i = 0; i < chipsize; i += 2) {
		data[i] = 0xFF;
		data[i+1] = 0x3F;
	}
	while (scanf(":%2x%4x%2x", &len, &addr, &type) == 3) {
		if (type == 1)
			break;
		while (len--) {
			if (scanf("%2x", &c) != 1) {
				printf("verify hex file sanity\n");
				return 1;
			}
			/* note stuff may exist out of program mem */
			/* thus we don't want to error on addr < chipsize */
			if (type == 0 & addr < chipsize)
				data[addr] = c;
			++addr;
		}
		scanf("%2x\n", &c); /* discard checksum */
	}
	/* make checksum */
	for (i = 0; i < chipsize - 4; ++i)
		sum = ( ((sum >> 1) | ((sum & 1) << 15)) + data[i] ) & 0xFFFF;
	printf(":041FFC00%02X34%02X34%2X\n", sum >> 8, sum & 0xFF, ((0xFF ^ (0x87 + (sum & 0xFF) + (sum >> 8))) + 1) & 0xFF);
	return 0;
}

