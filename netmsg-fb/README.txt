This is Netmsg version 1.0
============================

Netmsg is a two-way communications system that runs in a client-server fashion.
One machine run as the server, and another as a client. Netmsg is basically a
simple instant-messaging system.

The server will only handle one client. Any attempts to connect additional
clients will not work (they will connect, but the server has no means of 
attaching to them)

This program is currently Win32-specific, but would require VERY minor changes
for Linux support. There is no DOS support, but if I find a copy of the DOS
pthreads library, I might be able to convince the FreeBASIC maintainers to
use it for RTLib's DOS target. DOS support will also require Waterloo TCP.

This program is also very CPU and memory efficent. The total CPU usage 
for a 30 minute chat session (in seconds) is around 15 seconds for both 
client and server.

It might seem odd that the client and server act a lot alike. The client is
actually a modified version of the server's code.

To compile:

fbc serv.bas
fbc client.bas

RUNNING NETMSG
===========================

Netmsg is designed with a server-client model. Thus, the server machine must
be started before the client can connect. Please note that the client CAN
connect to the server as soon as it is started, but any messages sent by the
client to the server before the chat nickname is entered on the server are 
silently dropped. This is belived to be a bug in the server, but I don't feel
like fixing it.

BUG REPORTS
============================

Netmsg has a few graphical glitches at times, and most of the potential ones
are taken care of (prevented) by using mutexes and thread locking. On
occasion, the status bar will end up being shown twice, on the input entry 
line. This is not fatal, and will not affect the running of the program.

If something serious happens, please send a sensible and in-depth report to
segin2005@gmail.com. Send an email saying something "netmsg crashed, pls help"
tells me nothing. It's like finding a dead body. Sure, you know he's dead, but
you don't know how he died, or who you need to arrest for his murder, or even
if it was murder at all!


TEXT ENTRY (FOR ADVANCED USERS AND PROGRAMMERS)
============================

Netmsg uses a custom-made input entry loop. The loop is capable of managing
text slightly better than the traditional BASIC input statements, including
the ability to save the cursor position for incoming messages and restore it
after preforming the input check. Input is checked every 50 milliseconds. One
side effect of the save/restore of the cursor position is that the console 
caret will always appear in the message area. This can be confusing if you 
accidently type a space, and then leave your computer. It it probably possible
to emulate the proper cursor, it would probably break the code. If it ain't 
broke, don't fix it.