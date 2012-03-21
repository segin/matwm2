define abc msg "this is xmacro"
macro testmacro
	abc
endm
xmacro testxmacro
	abc
endm
define abc msg "this is normal macro"
testmacro
testxmacro
