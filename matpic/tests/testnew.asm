; test file for matpic C version
; all 14bit PIC instructions are here (hopefully
; and (almost) all directives etc should also be tested here

 ; testing various directives
 msg "Test message for message directive"

 ; testing include
 include "tests/testnew.inc"
 include "tests/pptest.asm"
 ifdef includesuccess
	return
	sleep
 endif

 ; testing ifdef and ifndef
 define apples
 ifdef apples
 	clrwdt
 endif
 ifdef bananas
 	syntax error
 endif
 ifndef bananas
 	option
 endif
 ifndef apples
	apples is not defined
 endif
 ifdef explode
	fdqsm{@#^#[{{@#{#@{}{@#{{}@#{€@#}[{sdfqqf
	qdfza"ra"ér)à"é r)" rà")é r; dqfdsfqd
	eslqfjopeioefzquefoiuqsdf
	define define define
	error error
	summon the dinosaurs
&é"'è()'"()'"è()'(")éé"é"é"""
 endif
 ifndef __test_asm__
 define __test_asm__
	nop
	; nesting must also work
	ifdef define
		movf 0x10, 1
		ifdef boobs
			are undefined
		endif
	endif
 endif
 ifndef __test_asm__
 	we failed
 endif

 ; test enum
enum 0, w, f

 ; testing all instructions
 addwf 0x7F, w
 andwf 0x7F, f
 clrf 0x7F
 clrw
 comf 0x7F, w
 decf 0x7F, f
 decfsz 0x7F, w
 incf 0x7F, f
 incfsz 0x7F, w
 org 0x50
 iorwf 0x7F, f
 movf 0x7F, w
 movwf 0x7F
 nop
 rlf 0x7F, w
 rrf 0x7F, w
 org 0x50 + 0x50
 subwf 0x7F, f
 swapf 0x7F, f
 xorwf 0x7F, f

 bcf 0x7F, 7
 bsf 0x7F, 7
 btfsc 0x7F, 7
 btfss 0x7F, 7
 bcf 0x7F, 7

 addlw 0b1111_1111 ; binary notation
 andlw 0xFF
 call 0x7FF
 ClRwDt ; testing cases (in)sensitiviy
 goto 0x7FF
 ; lets test enum again
  enum 15, a, b, c, enum, define
define enum 18
define define 19

 iorlw a
 movlw b
 retfie
 retlw c
 return
 sleep
 sublw enum
 xorlw define
 option
 tris 0x7
 ; testing data, and some math
 data 0x3FFF, 0, 0x1010, (((((((((5 + 5) * 5) - 3) * (4 + 3) / 3) + 6) ^ 1) | 0b11000) & 0b11011) << 2) >> 1, 10

