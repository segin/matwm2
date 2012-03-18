	include "include/p12f683.inc"
	include "tests/enum.inc"

	enum 0x60, spidata, ddata, ddata2, ddata3

	org 0
	goto start
	org 4 ; interrupt is here
	bcf intcon, intf
	btfsc pir1, tmr1if
	goto tick ; is timer interrupt
	retfie

tick
	movlw 0xC0
	iorwf tmr1h, f ; we only want 1/4th the time of filling tmr1h
	bcf pir1, tmr1if
	call dot
	comf 0x28, f
	btfss status, z
	call sec
	retfie
start

	; initialize hardware
	bcf status, 5 ; go bank 0
	movlw 0x07 ; cmcon is here
	movwf cmcon0
	bsf status, 5 ; go bank 1
	movlw 0b11111000
	andwf ansel, f
	movlw 0b11111000
	andwf trisio, f
	bcf status, 5 ; gpio crap is in bank 0

	; stop display test
	movlw 0x0F
	call spi_send
	movlw 0x00
	call spi_send
	call load

	; set brightness
	movlw 0x0A
	call spi_send
	movlw 0x0A ; 10
	call spi_send
	call load

	; set scan limit
	movlw 0x0B
	call spi_send
	movlw 0x07
	call spi_send
	call load

	; disable code B decoding
	movlw 0x09
	call spi_send
	movlw 0x00
	call spi_send
	call load

	rep 8
	clrf 0x40+@@
	endrep

	; as to show 88:88 before starting
	movlw 8
	movwf 0x24
	movwf 0x25
	movwf 0x26
	movwf 0x27
	call update
	call dot

	clrf 0x23
	clrf 0x24
	clrf 0x25
	clrf 0x26
	clrf 0x27
	clrf 0x28
	call update

	; disable shutdown mode
	movlw 0x0C
	call spi_send
	movlw 0x01
	call spi_send
	call load

; set up timer1
	; do what datasheet saids we need for timer interrupt
	bsf intcon, peie
	bsf intcon, gie
	bsf status, 5
	bsf pie1, tmr1ie
	bcf status, 5

	bsf t1con, tmr1cs ; set external clock (= t1osc)
	bsf t1con, t1oscen ; enable t1osc
	bcf t1con, tmr1ge ; disable tmr1 gate

	; set prescaler to 1:1
	bcf t1con, 4
	bcf t1con, 5

	; datasheet saids we need to clear these
	movlw 0xC0
	movwf tmr1h
	clrf tmr1l
	bcf pir1, tmr1if

	; turn the timer on
	bsf t1con, tmr1on

loop
	clrf 0x50 ; 'on' samples bitmap
	clrf 0x55 ; 'off' samples bitmap
	bcf intcon, gie

	clrf 0x51 ; sample 'on' counter
	bsf gpio, 0
	movlw 0xFF
	movwf 0x54 ; loop counter
	.inloop0:
		btfsc gpio, 3
		incf 0x51, f
		decfsz 0x54, f
		goto .inloop0
	bcf gpio, 0
	; check if we got a sample without 0s
	movf 0x51, w
	sublw 0xFF
	btfsc status, z
	bsf 0x50, 0
	; check if we got a sample without 1s
	movf 0x51, w
	sublw 0xFF
	btfss status, z
	bsf 0x55, 0

	clrf 0x51
	bsf gpio, 2
	movlw 0xFF
	movwf 0x54
	.inloop1:
		btfsc gpio, 3
		incf 0x51, f
		decfsz 0x54, f
		goto .inloop1
	bcf gpio, 2
	movf 0x51, w
	sublw 0xFF
	btfsc status, z
	bsf 0x50, 1
	movf 0x51, w
	sublw 0xFF
	btfss status, z
	bsf 0x55, 1

	movf 0x50, w
	xorwf 0x55, w
	sublw 2
	btfsc status, z
	goto loop ; we are unsure

	movf 0x50, w
	xorwf 0x52, w
	andwf 0x50, w
	movwf 0x53
	btfsc 0x53, 0
	call mnch
	btfsc 0x53, 1
	call hncm

	movf 0x50, w
	movwf 0x52
	bsf intcon, gie
	goto loop

sec
	movf 0x23, w
	sublw 59
	btfsc status, z
	goto min
	incf 0x23, f
	return
min
	clrf 0x23
	movf 0x24, w
	sublw 9
	btfsc status, z
	goto tenmin
	incf 0x24, f
endmin
	call update
	call display
	return
tenmin
	clrf 0x24
	movf 0x25, w
	sublw 5
	btfsc status, z
	goto hour
	incf 0x25, f
	goto endmin
hour
	clrf 0x25
hncm
	movf 0x26, w
	sublw 9
	btfsc status, z
	goto tenhour
	movf 0x26, w
	sublw 3
	btfsc status, z
	goto four
notwenty
	incf 0x26, f
	goto endmin
tenhour
	clrf 0x26
	movf 0x27, w
	incf 0x27, f
	goto endmin
four
	movf 0x27, w
	sublw 2
	btfss status, z
	goto notwenty
	clrf 0x26
	clrf 0x27
	goto endmin
mnch
	clrf 0x23
	movf 0x24, w
	sublw 9
	btfsc status, z
	goto tenminnch
	incf 0x24, f
	goto endmin
tenminnch
	clrf 0x24
	movf 0x25, w
	sublw 5
	btfsc status, z
	goto clearmin
	incf 0x25, f
	goto endmin
clearmin
	clrf 0x24
	clrf 0x25
	goto endmin

; 0x24-0x27 = digits
update
	movlw 4
	movwf 0x21
	movf 0x27, w
	call decsetdigit
	movlw 3
	movwf 0x21
	movf 0x26, w
	call decsetdigit
	movlw 2
	movwf 0x21
	movf 0x25, w
	call decsetdigit
	movlw 1
	movwf 0x21
	movf 0x24, w
	call decsetdigit
	return

dot
	comf 0x47, f
	call display
	return

; w = byte to decode
; destroys register 0x21-0x22, 0x30-0x37 and 0x40-0x47
; 0x21 digit #
decsetdigit
	bsf status, 5 ; bank 1 for eeprom stuff
	movwf eeadr
	bsf eecon1, rd
	movf eedat, w
	bcf status, 5 ; back to bank 0
	; fallthrough
; w = byte to decode
; 0x21 digit #
; destroys register 0x22, 0x30-0x37 and 0x40-0x47
setdigit
	movwf 0x20
	rep 8
		clrf 0x30+@@
	endrep
	rep 8
		btfsc 0x20, @@
		bsf 0x30+@@, 0
	endrep
	movf 0x21, w
roll
	decfsz 0x21, f
	goto rollon
	goto combine
rollon
	rep 8
		rlf 0x30+@@, f
	endrep
	goto roll
combine
	; shift 1 left to match digit
	movwf 0x21
	movlw 1
	movwf 0x22
combine2
	rlf 0x22, f
	decfsz 0x21, f
	goto combine2
	rrf 0x22, f
	comf 0x22, f
	rep 7
		movf 0x22, w
		andwf 0x40+@@, f
		movf 0x30+@@, w
		iorwf 0x40+@@, f
	endrep
	movf 0x22, w
;	andwf 0x47, f ; why does this fuck with dot?
	movf 0x37, w
	iorwf 0x47, f
	return

display
	rep 8
		movlw 0x01+@@
		call spi_send
		movf 0x40+@@, w
		call spi_send
		call load
	endrep
	return

load
	bsf gpio, 2
	bcf gpio, 2
	return

; spi_send(byte)
;  w is byte to send
;  gpio 0 is data
;  gpio 1 is clock
spi_send
	define _spibit 0x80
	rep 8
		if @@ == 0
			movwf spidata
		else
			movf spidata, w
		endif
		andlw _spibit
		btfss status, z
		bsf gpio, 0
		call clock
		xdefine _spibit [_spibit >> 1]
	endrep
	return

clock
	bsf gpio, 1
	bcf gpio, 1
	bcf gpio, 0
	return

	org 0x2007
	data _INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_ON & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _FCMEN_OFF & _IESO_OFF

	org 0x2100
	data 0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101
	data 0b01111101, 0b00000111, 0b01111111, 0b01101111, 0
