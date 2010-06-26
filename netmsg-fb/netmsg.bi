'
' Protocol definition
'

#ifndef __NETMSG_BI__
#define __NESMSG_BI__ 1

Type proto
	Type As string * 5
	UName as string * 31
	Msg as string * 257
End Type 

' This only defined a header, but that's better than nothing.
' This will allow for a more flexible protocol that's more realistic
' yet still compatable in design as the original "block" protocol.
' The main problem with the original proto is the implementation.
'
' The data packet must be sent in a single TCP/IP packet. 
' In the near future, I will design a "connectionless" server
' using datagrams over UDP/IP.
'

Type proto_ng
	Magic As Integer
	Type As Integer
	User As Integer
	Msg As Integer
End Type

#define MAKEMSG(a0,a1,a2,a3) asc(a0) + asc(a1) shl 8 + asc(a2) shl 16 + asc(a3) shl 24

const SERVER_MSG_HELLO	= MAKEMSG( "H", "E", "L", "O" )
const SERVER_MSG_SUP	= MAKEMSG( "S", "U", "P", "!" )
const SERVER_MSG_CHAT	= MAKEMSG( "C", "H", "A", "T" )
const SERVER_MSG_BYE	= MAKEMSG( "B", "Y", "E", "!" )
const SERVER_MSG_CYA	= MAKEMSG( "C", "Y", "A", "!" )

#endif 
