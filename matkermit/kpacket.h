#ifndef __KPACKET_H__
#define __KPACKET_H__

/* Absolute maximum length for kermit packet we will use */
/* 4 bytes header + max data (94) + 1 byte checksum + \r\n + NUL = 102 */
#define KPACKET_MAXLEN  102

/* Response types */
#define KPACKET_TYPE_DATA 'D' /* Data packet */
#define KPACKET_TYPE_ACK  'Y' /* Acknowledgement */
#define KPACKET_TYPE_NAK  'N' /* Negative acknowledgement */
#define KPACKET_TYPE_SEND 'S' /* Send initiative (exchange parameters) */
#define KPACKET_TYPE_EOT  'E' /* Break transmission */
#define KPACKET_TYPE_FILE 'F' /* File header */
#define KPACKET_TYPE_EOF  'Z' /* End of file */
#define KPACKET_TYPE_ERR  'E' /* Error */
#define KPACKET_TYPE_ATTR 'A' /* Optional attribute packet */
/* note that Q and T are reserved */

/* Server commands */
#define KPACKET_TYPE_RECV 'R' /* Ask for receiving a file */
#define KPACKET_TYPE_INIT 'I' /* Initialize (exchange parameters) */
#define KPACKET_TYPE_TEXT 'X' /* Text header (response of command) */
#define KPACKET_TYPE_CMD  'C' /* Run host command */
#define KPACKET_TYPE_KERM 'K' /* Run kermit command */
#define KPACKET_TYPE_GEN  'G' /* Generic kermit command */
/* Generic kermit commands
 *  in comments before - operands are described
 *  Optional operands are in []
 *  Operands are prefixed with 1 byte specifying their length
 *  With arguments in the form [a][b] i believe a needs length 0 if not given
 */
#define KPACKET_SCMD_LOGI 'I' /* [user[pass[acct]]] - login */
#define KPACKET_SCMD_CWD  'C' /* [directory[password]] - change directory */
#define KPACKET_SCMD_LOGO 'L' /* - logout */
/* these are optional */
#define KPACKET_SCMD_FINI 'F' /* - finish (shutdown the server but no logout) */
#define KPACKET_SCMD_DIR  'D' /* [filespec] - query directory listing */
#define KPACKET_SCMD_DUSG 'U' /* [area] - disk usage query */
#define KPACKET_SCMD_DEL  'E' /* filespec - delete */
#define KPACKET_SCMD_TYPE 'T' /* filespec - type */
#define KPACKET_SCMD_REN  'R' /* oldname,newname - rename */
#define KPACKET_SCMD_COPY 'K' /* source,destination - copy */
#define KPACKET_SCMD_WHO  'W' /* [uid_or_host[options]]- who is logged in */
#define KPACKET_SCMD_MSG  'M' /* destination,text - send short message */
#define KPACKET_SCMD_HELP 'H' /* [topic] - ask for help */
#define KPACKET_SCMD_STAT 'Q' /* - query server status */
#define KPACKET_SCMD_PROG 'P' /* [filespec][commands] - program */
#define KPACKET_SCMD_JOUR 'J' /* command[argument] - journal */
#define KPACKET_SCMD_VAR  'V' /* command[argument[argument]] - variable */

#define tochar(x) ((x) + 32)
#define unchar(x) ((x) - 32)
#define ctl(x)    ((x) ^ 64)

#endif /* __KPACKET_H__ */

