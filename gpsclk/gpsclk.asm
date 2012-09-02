#include "16f690.inc"
#include "alloc.inc"
#include "print.inc"

	alloc recvdata, 72
	alloc tmrflags
	alloc tmp0 ; used by lcd stuff
	alloc tmp1 ; used by print stuff
	alloc tmp2
	alloc tmp3
	define gtmp 0x70 ; for interrupt to store W, status, etc
	; 0x71, 0x72 also used by interrupt handler

	org 0x2007
	; 0-2 hs osc
	; 3 wdte on
	; 4 pwrt on
	; 5 mclre on
	; 6 cp off
	; 7 cpd off
	; 8-9 bor on
	; 10 ieso on
	; 11 fcmen on
	; 12-13 reserved (must be 1)
	data 0b11_1111_1111_0010

	org 0
	goto start
	org 4
	; save state
	movwf gtmp ; store W
	movf STATUS, W
	movwf gtmp+1 ; store STATUS
	movf PCLATH, W
	movwf gtmp+2
	clrf PCLATH
	; do shit
	banksel 0
	btfsc INTCON, T0IF
	call tmr0flow
	btfsc PIR1, TMR1IF
	call tmr1flow
	btfsc PIR2, OSFIF
	call oscfail
	; restore state
	movf gtmp+2, W
	movwf PCLATH
	movf gtmp+1, W
	movwf STATUS
	swapf gtmp ; swap so we can restore with a swapf that does not affect flags
	swapf gtmp, W ; restore our swapped W not affecting any flags :)
	retfie

oscfail
	banksel 0
	showmsg string.eosc
	goto $

tmr0flow
	bsf tmrflags, 0
	bcf INTCON, T0IF
	return

tmr1flow
	bsf tmrflags, 1
	bcf PIR1, TMR1IF
	return

start: call init

	; enable uart receiver
	call rcv_enable

	; request status
	movlw '@'
	call xmit
	movlw '@'
	call xmit
	movlw 'E'
	call xmit
	movlw 'a'
	call xmit
	movlw 1
	call xmit
	movlw 'E'^'a'^1
	call xmit
	movlw '\r'
	call xmit
	movlw '\n'
	call xmit

loop
	movlw 1
	call recv
	movlw '@'
	subwf recvdata
	btfss STATUS, Z
	goto loop
	movlw 3
	call recv
	movlw '@'
	subwf recvdata
	btfss STATUS, Z
	movlw 'E'
	subwf recvdata+1
	btfss STATUS, Z
	goto loop
	movlw 'a'
	subwf recvdata+2
	btfss STATUS, Z
	goto loop
	movlw 72
	call recv

	movlw 0x80
	call lcd_cmd
	movlw ' '
	call lcd_char
	movf recvdata+4, W
	call printnum8
	movlw ':'
	call lcd_char
	movf recvdata+5, W
	call printnum8
	movlw ':'
	call lcd_char
	movf recvdata+6, W
	call printnum8
	movlw ' '
	call lcd_char
	movf recvdata+1, W
	call printnum8
	movlw '/'
	call lcd_char
	movf recvdata+0, W
	call printnum8
	movlw ' '
	call lcd_char

	movlw 0xC0
	call lcd_cmd
	movlw 'n'
	call lcd_char
	movlw '='
	call lcd_char
	movf recvdata+34, W
	call printnum8
	movlw '/'
	call lcd_char
	movf recvdata+35, W
	call printnum8
	movlw ' '
	call lcd_char
	movlw ' '
	call lcd_char
	movlw '\xE5' ; place holder for when time RAIM enabled later
	call lcd_char
	movlw '='
	call lcd_char
	movlw '6'
	call lcd_char
	movlw '5'
	call lcd_char
	movlw '5'
	call lcd_char
	movlw '3'
	call lcd_char
	movlw '5'
	call lcd_char

	call lcd_init
	clrwdt
	goto loop

#include "init.asm"
#include "tmr0.asm"
#include "tmr1.asm"
#include "uart.asm"
#include "lcd.asm"
#include "print.asm"
#include "strings.asm"
#include "selftest.asm" ; must be last!
