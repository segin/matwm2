/* 
 * NetMSG Protocol, C Language Implementation 
 * Copyright (C) 2008-2009 Kirn Gill <segin2005@gmail.com>
 *  
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE. 
 */

#ifndef __NETMSG_H__
#define __NETMSG_H__
struct netmsg_protocol {
	char p_type[6]; /* Last two bytes should be NUL */
	char p_user[32];
	char p_msg[258];
} netmsg_protocol;

struct {
	int p_type;
	int p_user;
	int p_msg;
#ifdef __GNUC__
	char data[0]; /* GNU C Extension */
#endif
} netmsg_protocol_ng;

#define MAKEMSG(a0,a1,a2,a3) (a0) + (a1 << 8) + (a2 << 16) + (a3 << 24)

struct {
	int v_major; /* Major version. Version 2 will use protocol_ng */
	int v_minor; /* Minor version */
	int v_patch; /* if nightly, a date such as 20090112 */
	char v_string[12]; /* Stringized version. Report *THIS*. */
	char c_hostos[12]; /* Determine via uname() or a static string. */
	char c_harch[12]; /* Host arch */
	char c_hosrel[12]; /* Host OS version. Get via uname(). */
	char c_implnt[20]; /* Language in which protocol is implented in */
} netmsg_client_info;

const int PROTO_MSG_HELLO	= MAKEMSG( 'H', 'E', 'L', 'O' );
const int PROTO_MSG_SUP		= MAKEMSG( 'S', 'U', 'P', '!' );
const int PROTO_MSG_CHAT	= MAKEMSG( 'C', 'H', 'A', 'T' );
const int PROTO_MSG_BYE		= MAKEMSG( 'B', 'Y', 'E', '!' );
const int PROTO_MSG_CYA		= MAKEMSG( 'C', 'Y', 'A', '!' );
const int PROTO_MSG_INFO	= MAKEMSG( 'I', 'N', 'F', 'O' );

#endif /* __NETMSG_H__ */
