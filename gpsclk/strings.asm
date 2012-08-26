	alloc strhi
	alloc strlo

string
	movf strhi, W
	movwf PCLATH
	movf strlo, W
	movwf PCL
.starting1
	retlw 'M'
	retlw 'A'
	retlw 'T'
	retlw ' '
	retlw ' '
	retlw 'C'
	retlw 'o'
	retlw 'r'
	retlw 'p'
	retlw 'o'
	retlw 'r'
	retlw 'a'
	retlw 't'
	retlw 'i'
	retlw 'o'
	retlw 'n'
	retlw 0
.starting2
	retlw ' '
	retlw 'G'
	retlw 'P'
	retlw 'S'
	retlw ' '
	retlw 'R'
	retlw 'e'
	retlw 'c'
	retlw 'e'
	retlw 'i'
	retlw 'v'
	retlw 'e'
	retlw 'r'
	retlw ' '
	retlw '1'
	retlw ' '
	retlw 0
.eframe
	retlw 'U'
	retlw 'A'
	retlw 'R'
	retlw 'T'
	retlw ' '
	retlw 'f'
	retlw 'r'
	retlw 'a'
	retlw 'm'
	retlw 'e'
	retlw ' '
	retlw 'e'
	retlw 'r'
	retlw 'r'
	retlw 'o'
	retlw 'r'
	retlw 0
.eovfl
	retlw ' '
	retlw 'U'
	retlw 'A'
	retlw 'R'
	retlw 'T'
	retlw ' '
	retlw ' '
	retlw 'o'
	retlw 'v'
	retlw 'e'
	retlw 'r'
	retlw 'f'
	retlw 'l'
	retlw 'o'
	retlw 'w'
	retlw ' '
	retlw 0
.eosc
	retlw 'M'
	retlw 'C'
	retlw 'U'
	retlw ' '
	retlw 'c'
	retlw 'l'
	retlw 'o'
	retlw 'c'
	retlw 'k'
	retlw ' '
	retlw 'f'
	retlw 'a'
	retlw 'i'
	retlw 'l'
	retlw 'e'
	retlw 'd'
	retlw 0

