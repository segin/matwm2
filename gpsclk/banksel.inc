ifndef __banks
	define __banks 4
endif

macro banksel reg
	if reg & 0x80
		bsf 3, 5
	else
		bcf 3, 5
	endif
	if __banks > 2
		if reg & 0x100
			bsf 3, 6
		else
			bcf 3, 6
		endif
	endif
endm

macro bankisel reg
	if reg & 0x100
		bsf 3, 7
	else
		bcf 3, 7
	endif
endm
