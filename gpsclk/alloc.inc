macro alloc name
	ifndef __alloc_pos
		define __alloc_pos 0x20
	endif
	name: equ __alloc_pos
	if @0 == 0
		xdefine __alloc_pos [__alloc_pos+1]
	else
		xdefine __alloc_pos [__alloc_pos+@1]
	endif
endm

