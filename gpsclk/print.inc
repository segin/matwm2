macro print addr
	movlw (addr >> 8)
	movwf strhi
	movlw addr
	movwf strlo
	call _print
endm

macro showmsg addr
	movlw (addr >> 8)
	movwf strhi
	movlw addr
	movwf strlo
	call _showmsg
endm

