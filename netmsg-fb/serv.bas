'
' Network messaging server.
' Copyright (C) 2007, Segin<segin2005@gmail.com>
'
' Licensed under the GPLv2
'
' Beware, heavy wizardry at work!
'

#include once "netmsg.bi"

#ifdef __FB_WIN32__
#include "win/winsock2.bi"
#endif
#ifdef __FB_DOS__
#error "Please provide info on Waterloo TCP/IP to maintainer"
#endif
#if defined(__FB_LINUX__) or defined(__FB_FREEBSD__)
#include once "crt/netinet/in.bi"
#include once "crt/arpa/inet.bi"
#include once "crt/netdb.bi"
#include once "crt/sys/socket.bi"
#include once "crt/errno.bi"
#define TRUE	1
#define FALSE	0
'Linux console is not appropriate for attempting to get input due to
'the fact it is a data stream, and there's no API to get things like
'keycodes, only hacks. Pity.
'
'This does have advantages over Windows, though.
'
'gfxlib also supports the use of a framebuffer so X11 is not needed 
'to be running, only installed.
'
'Please note that running both the server and client on a Linux box sends CPU
'usage up a LOT, on muy machine, to about 50%. 40% of it is X. It's cause of
'an inefficency in X11 or gfxlib. Could be my text entry loop. Would someone
'care to rewrite it to use events without changing it much?
screen 12
#endif 

#ifdef __FB_WIN32__
function hStart( byval verhigh as integer = 2, byval verlow as integer = 0 ) as integer
	dim wsaData as WSAData
	
	if( WSAStartup( MAKEWORD( verhigh, verlow ), @wsaData ) <> 0 ) then
		return FALSE
	end if
	
	if( wsaData.wVersion <> MAKEWORD( verhigh, verlow ) ) then
		WSACleanup( )	
		return FALSE
	end if
	
	function = TRUE

end function
function hShutdown( ) as integer

	function = WSACleanup( )
	
end function
#define hPrintError(e) print "error calling "; #e; ": "; WSAGetLastError( )

#endif
#ifdef __FB_LINUX__
'
' Linux does NOT need socket library inits, unlike Windows...
'
function hStart() as integer
	return TRUE
end function

function hShutdown() as Integer
	return hStart()
end function
#define hPrintError(e) print "error calling "; #e; "."  
#endif
function hResolve( byval hostname as string ) as integer
	dim ia as in_addr
	dim hostentry as hostent ptr

	'' check if it's an ip address
	ia.S_addr = inet_addr( hostname )
	if ( ia.S_addr = INADDR_NONE ) then
		
		'' if not, assume it's a name, resolve it
		hostentry = gethostbyname( hostname )
		if ( hostentry = 0 ) then
			exit function
		end if
		
		function = *cast( integer ptr, *hostentry->h_addr_list )
		
	else
	
		'' just return the address
		function = ia.S_addr
	
	end if
	
end function

'':::::
function hOpen( byval proto as integer = IPPROTO_TCP ) as SOCKET
	dim s as SOCKET
    
    s = opensocket( AF_INET, SOCK_STREAM, proto )
    if( s = NULL ) then
		return NULL
	end if
	
	function = s
	
end function

'':::::
function hConnect( byval s as SOCKET, byval ip as integer, byval port as integer ) as integer
	dim sa as sockaddr_in

	sa.sin_port			= htons( port )
	sa.sin_family		= AF_INET
	sa.sin_addr.S_addr	= ip
	
	function = connect( s, cast( PSOCKADDR, @sa ), len( sa ) ) <> SOCKET_ERROR
	
end function

'':::::
function hBind( byval s as SOCKET, byval port as integer ) as integer
	dim sa as sockaddr_in

	sa.sin_port			= htons( port )
	sa.sin_family		= AF_INET
	sa.sin_addr.S_addr	= INADDR_ANY 
	
	function = bind( s, cast( PSOCKADDR, @sa ), len( sa ) ) <> SOCKET_ERROR
	
end function

'':::::
function hListen( byval s as SOCKET, byval timeout as integer = SOMAXCONN ) as integer
	
	function = listen( s, timeout ) <> SOCKET_ERROR
	
end function

'':::::
function hAccept( byval s as SOCKET, byval sa as sockaddr_in ptr ) as SOCKET
	dim salen as integer 
	
	salen = len( sockaddr_in )
	function = accept( s, cast( PSOCKADDR, sa ), @salen )

end function	

'':::::
function hClose( byval s as SOCKET ) as integer

	shutdown( s, 2 )
	
	#ifdef __FB_WIN32__
	function = closesocket( s )
	#endif
	#if defined(__FB_LINUX__) or defined(__FB_FREEBSD__)
	function = close(s)
	#endif
	
end function

'':::::
function hSend( byval s as SOCKET, byval buffer as zstring ptr, byval bytes as integer ) as integer

    function = send( s, buffer, bytes, 0 )
    
end function

'':::::
function hReceive( byval s as SOCKET, byval buffer as zstring ptr, byval bytes as integer ) as integer

    function = recv( s, buffer, bytes, 0 )
    
end function

'':::::
function hIp2Addr( byval ip as integer ) as zstring ptr
	dim ia as in_addr
	
	ia.S_addr = ip
	
	function = inet_ntoa( ia )

end function

#define CLIENTADDR(c) *hIp2Addr( c.sin_addr.S_addr ) & "(" & c.sin_addr & ")"


Dim shared sock As SOCKET
Dim ret As Integer
Dim Shared user As ZString * 30
Dim shared packet as Proto
Dim msg As ZString * 256
Dim shared sa As sockaddr_in
Dim shared s As SOCKET
Dim char As Byte
Dim shared nick As String
Dim Shared Connected As Integer
Dim Threads(2) as Any Ptr 
Dim Shared Running As Integer
Dim Shared mutex As Any Ptr

const SERVER_PORT = 1337

cls
 
function serverIni( ) as integer

	'' start winsock
	if( hStart( ) = FALSE ) then
		hPrintError( hStart )
		return FALSE
	end if
	
	'' create a socket for listening
	sock = hOpen( )
	if( sock = NULL ) then
		hPrintError( hOpen )
		return FALSE
	end if
	
	'' bind it to the server port
	if( hBind( sock, SERVER_PORT ) = FALSE ) then
		hPrintError( hBind )
		return FALSE
	end if	
	
	function = TRUE
	
end function

function GetString(Prompt As String = "> ") As String
	Dim char As Byte
	Dim Path As String
	Dim X As Integer
	Dim Y As Integer
	Dim TY as Integer
	Dim TX As Integer
	MutexLock(mutex)
	X = CsrLin()
	Y = Pos()
	Locate 1,1
	Print Space(80);
	Locate 1,1
	Print Prompt;
	MutexUnlock(mutex)
	do while char <> 13
	char = Asc(inkey$)
	if char > 31 And char < 127 then 
		If Len(Path) = 255 Then Goto StartScreenUpdate
		If Len(Path) > 255 Then Path = Left(Path,255)
		Path+=Chr(char)
		Goto StartScreenUpdate
	end if 
	if char = 8 Then
		Path = Left(Path, Len(Path) - 1)
		Goto StartScreenUpdate
	End If
	
	StartScreenUpdate:
	MutexLock(mutex)
		' Screenlock to prevent flicker
		' Useless in console modes
		ScreenLock
			Locate 1,1
			Print Space(80);
			Locate 1,1
			Print Prompt;
			Locate 1,1+Len(Prompt)
			Print Right(Path,80 - Len(Prompt));
		ScreenUnlock
	EndScreenUpdate:
	TY = Pos()
	TX = CsrLin() 
	Locate X, Y
	sleep 50
	X = CsrLin
	Y = Pos()
	Locate TX, TY
	MutexUnlock(mutex)
	loop
	Locate 1,1
	Print Space(80);
	Locate X, Y
	Return Path
End Function

Sub Quit(ret As Integer = 0)
	MutexDestroy(mutex)
	End ret
End Sub

Sub UpdateStatusBar() 
	MutexLock(mutex)
	Dim X As Integer
	Dim Y As Integer
	X = CsrLin()
	Y = Pos()
	Locate 2,1
	Color 0,7
	Print Space(80)
	Locate 2,2
	Print "-- Connection from " + *hIp2Addr( sa.sin_addr.S_addr ) + " (" + nick + ") [server] --"
	Color 7,0
	Locate X, Y
	MutexUnlock(mutex)
End Sub

Color 7,0
mutex = MutexCreate
Running = 1
ret = serverIni( )
if ret = FALSE then
	print "Error in netmsgd server init"
	hShutdown
	Quit
end if
if( hListen( sock ) = FALSE ) then
	hPrintError( hListen )
	hShutdown
	Quit FALSE
end if

locate 3,1
UpdateStatusBar() 

Nickname:
Print "Please enter a nick." 
msg = GetString("nick> ")
nick = Left(msg,30)
if Len(nick) = 0 goto Nickname
UpdateStatusBar() 
Print "Using nickname " + nick
print "Waiting for connection on port 1337"

s = hAccept( sock, @sa )
if( s = -1 ) then
	hPrintError( hAccept )
end if
UpdateStatusBar() 
print "Connection from " + *hIp2Addr( sa.sin_addr.S_addr ) + "(" &  sa.sin_port & ")"
Connected = 1

Sub SendProtoMsg(mType As String, msg As String)
	MutexLock(mutex)
	If Len(msg) = 0 Then Exit Sub
	packet.msg = msg
	packet.type = mType
	packet.uname = Cast(String, Left(Nick,30))
	hSend(s, @packet, Sizeof(packet))
	Color 2 
	if mType = "CHAT" then
		Print packet.uname + ": " + packet.msg
	End If
	color 7
	MutexUnlock(mutex)
	UpdateStatusBar() 
End Sub

Sub RecvThread()
	Dim msg As Proto
	Dim bytes As Integer
	Dim msgType As String
	Dim msgUser As String
	Dim msgMsg As String
	Dim i as Integer
	Do
	bytes = hReceive( s, @packet, Sizeof(packet) )
	msgUser = Str(bytes)
	if bytes <> Sizeof(packet) Then 
		If bytes = -1 Or bytes = 0 Then
			'Server has died without telling us.
			print "Connection closed by client."
			Connected = 0
			cls
			hShutdown
			quit		
			Goto EndThread	
		End If
		print "hRecieve() returned " & bytes & " bytes."
		Goto EndRecv
	End If
	Select Case packet.type
		Case "CHAT"
			' This is the only screen operation IN threaded section
			' of program execution in which locking mutex = bad stuff
			Color 6 
			Print packet.uNAME + ": " + packet.msg
			color 7
			UpdateStatusBar() 
		Case "BYE!" 
			SendProtoMsg("CYA!","server to client: clear to disconnect.")
			Connected = 0
			cls
			hShutdown
			quit
			Goto EndThread	
		Case else
			print !"Unknown packet type:\"" + packet.type + !"\"." 
			Print "Sizeof(packet) = " & Sizeof(packet) & ", bytes = " & bytes
			Sleep(1)
		
	End Select 
	EndRecv:
	Loop	
	EndThread:
	Running = 0	
End Sub


Threads(2) = ThreadCreate(@RecvThread)

Do While Connected
msg = GetString
if Left(msg, 5) = "/quit" Then 
	SendProtoMsg("CYA!","server to client: please disconnect now.") 
	Connected = 0
	Goto EndSend
End If
If Len(msg) > 1 Then
	SendProtoMsg("CHAT", msg)
End If
EndSend:
Loop
Running = 0
hClose(s)
hClose(sock)
hShutdown()
cls
quit
