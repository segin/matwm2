; test argument format f,d,a
addwf 0xFF, 0, 0
addwf 0x00, 0, 1
addwf 0xFF, 1, 0
addwf 0x00, 1, 1
addwf 0xFF, 0
addwf 0x00, 1
addwf 0xFF
addwf 0

;test argument format f,a
clrf 0xFF, 1
clrf 0xFF, 0
clrf 0xFF
clrf 0

; f,f
movff 0xFFF, 0
movff 0, 0xFFF

; f,b,a
bcf 0xFF, 0
bcf 0, 7, 0
bcf 0, 0

; n (8 and 11 bits)
bra 0x7FF
bra 0x700
bra 0x0FF
bc 0xFF
bc 0

; k (4, 8, 20, 'k,s' and 'f,k')
movlb 0x0F
movlb 0
addlw 0xFF
addlw 0
goto 0xFFFFF
goto 0
call 0xFFFFF
call 0xFFFFF, 1
call 0
lfsr 0, 0xFFF
lfsr 3, 0

; retfie and return take an optional 1 bit argument
retfie 0
return 1
retfie
return

; these should be nops
data 0xF000 ; nop
data 0xFFFF ; nop

; try all instructions in same order of datasheet
addwf 0x1
addwfc 0x2
clrf 0x3
comf 0x4
cpfseq 0x5
cpfsgt 0x6
cpfslt 0x7
decf 0x8
decfsz 0x9
dcfsnz 0xA
incf 0xB
incfsz 0xC
infsnz 0xD
iorwf 0xE
movf 0xF
movff 0x876, 0x987
movwf 0x10
mulwf 0x20
negf 0x30
rlcf 0x40
rlncf 0x50
rrcf 0x60
rrncf 0x70
setf 0x80
subfwb 0x90
subwf 0xA0
subwfb 0xB0
swapf 0xC0
tstfsz 0xD0
xorwf 0xE0

bcf 0xF0, 0
bsf 0, 0
btfsc 0, 0
btfss 0, 0
btg 0, 0

bc 0
bn 0
bnc 0
bnn 0
bnov 0
bnz 0
bov 0
bra 0
bz 0
call 0
clrwdt
daw
goto 0
nop
data 0xF000 ; nop too
pop
push
rcall 0
reset
retfie
retlw 0
return
sleep

addlw 0
andlw 0
iorlw 0
lfsr 0, 0
movlb 0
movlw 0
mullw 0
retlw 0
sublw 0
xorlw 0

tblrd
tblrdi
tblrdd
tblrdp
tblwt
tblwti
tblwtd
tblwtp

addfsr 0, 0
addulnk 0
callw
movsf 0, 0
movss 0, 0
pushl 0
subfsr 0, 0
subulnk 0
