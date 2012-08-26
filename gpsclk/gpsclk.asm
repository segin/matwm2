#include "16f690.inc"
#include "alloc.inc"
#include "print.inc"

	alloc recvdata, 80
	alloc tmp0
	alloc tmp1
	alloc tmp2
	alloc tmp3
	alloc tmp4
	alloc dc0
	alloc dc1
	define gtmp 0x70 ; for interrupt to store W, status, etc

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
	data 0b11_1111_1111_1010

	org 0
	goto start
	org 4
	movwf gtmp; store W
	movf STATUS, W
	movwf gtmp+1 ; store STATUS
	banksel 0
	btfsc PIR2, OSFIF
	call oscfail
	movf gtmp+1, W
	movwf STATUS
	movf gtmp, W
	retfie

oscfail
	banksel 0
	showmsg string.eosc
	banksel OSCCON
	goto $

start: call init

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

	movlw 0x80
	call lcd_cmd
loop
	movlw 1
	call recv
	movlw '@'
	subwf recvdata
	btfss STATUS, Z
	goto loop
	movlw 3
	call recv
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
	call lcd_char
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

	clrwdt
	goto loop

delay
	clrf dc0
_delay
	decfsz dc0
	goto _delay
	return

longdelay
	clrf dc1
_ldelay
	call delay
	decfsz dc1
	goto _ldelay
	return

#include "init.asm"
#include "uart.asm"
#include "lcd.asm"
#include "print.asm"
#include "strings.asm"

