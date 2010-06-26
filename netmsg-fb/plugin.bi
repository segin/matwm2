'
' Plugin API
'

#ifndef __PLUGIN_BI__
#define __PLUGIN_BI__

' Only supports 5 plugins
Type plugin_hooks
	number As Integer
	Type(5) As String * $
	Hook(5) As Sub Ptr
	Thread As Sub Ptr		
End Type

#endif 
