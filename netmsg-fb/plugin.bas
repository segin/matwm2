'
' Netmsg example plugin -- defines proxy protocol
'

#include "plugin.bi"
#include "netmsg.bi"

Function proxy_msg() As Any Ptr

End Function

ByRef Function plugin_init(ByVal ver As Integer) As plugin_hooks
	Function.number = 1 
	Function.Type(0) = "BACK"
	Function.Hook(0) = @proxy_msg
End Function
