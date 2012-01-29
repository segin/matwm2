 ; reference test file for gpasm
 ; assembled result will differ slightly for matpic
 ; 000009:  0103  clrw
 ; first two bits are not cared for according to microchip's datasheets
 ; gpasm uses ones, datasheet says the assembler uses 0s

 iorwf 2,1
 return
 sleep
 clrwdt
 option
 nop
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
 addlw 0xFF
 andlw 0xFF
 call 0x7FF
 clrwdt
 goto 0x7FF
 iorlw d'15'
 movlw d'16'
 retfie
 retlw d'17'
 return
 sleep
 sublw d'18'
 xorlw d'19'
 option
 tris 0x7
 data 0x3FFF, 0, 0x1010, 0x34, d'10'
 end
