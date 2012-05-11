#include "kpacket.h"

/* kpacket_fill
 *
 * description
 *   fills kermit packet
 * input
 *   len: data length in bytes (not including packet stuff), 0-94
 *   seq: sequence number
 * return value
 *   the length of the finished packed not including terminating 0
 * notes
 *   packet must have KPACKET_MAXLEN bytes allocated
 *   data field is not encoded by this function by itself in any way
 */
int kpacket_fill(char *packet, int seq, int type, char *data, int len) {
	int i, s = 0;
	packet[0] = 01;
	s += packet[1] = tochar(len + 3);
	s += packet[2] = tochar(seq % 64);
	s += packet[3] = type;
	for (i = 0; i < len; ++i) {
		s += data[i];
		packet[i+4] = data[i];
	}
	i += 4;
	packet[i] = tochar((s + ((s & 192) / 64)) & 63);
	packet[++i] = '\r';
	packet[++i] = '\n';
	packet[++i] = 0;
	return i;
}

