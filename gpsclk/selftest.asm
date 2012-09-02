org 0xF00

cshi equ tmp2 ; bank0
cslo equ tmp3
endd equ 0xFFE ; end of program data, two retlw should go there with checksum

; Makes a 16-bit checksum of the program memory until 'endd' address.
; Then verifies it from 2 retlw instructions (high first) after 'endd'.
;
; Algorithm is as follows: ((sum>>1)+((sum&1)<<15)+byte)&0xFFFF.
; Adding the high bytes first.
;
; input:
;   endd: adress where to stop (starts from address 0)
; output:
;   cshi (bank0): result high byte
;   cslo (bank0): result low byte
;   STATUS: carry set when failure
; destroys:
;   W, STATUS, EECON1, EEDAT, EEADR, EEDATH, EEADRH
;   will be in bank0 at return time
; notes:
;   put this function at last page to be checked
checkrom
	banksel 0
	clrf cshi ; checksum high byte
	clrf cslo ; checksum low byte
.loop
	banksel EECON1
	bsf EECON1, EEPGD
	bsf EECON1, RD
	nop ; while reading program memory we must nop
	nop ; ... twice
	banksel EEDAT
	movf EEDAT, W
	call csum_add
	banksel EEDAT
	movf EEDATH, W
	call csum_add
	banksel EEADR
	incf EEADR
	btfsc STATUS, Z
	incf EEADRH
	movf EEADR, W
	sublw endd & 0xFF ; leave 2 last unchecked so can store checksum there
	btfsc STATUS, Z
	goto .fd
	goto .loop
.fd
	movf EEADRH, W
	sublw endd >> 8
	btfss STATUS, Z
	goto .loop
	banksel 0
	call endd
	subwf cshi, W
	btfss STATUS, Z
	goto .fail
	call endd+1
	subwf cslo, W
	btfss STATUS, Z
	goto .fail
	bcf STATUS, C
	return
.fail
	bsf STATUS, C
	return

csum_add
	banksel 0
	bcf STATUS, C
	rrf cshi
	rrf cslo
	btfsc STATUS, C
	bsf cshi, 7
	addwf cslo
	btfsc STATUS, C
	incf cshi
	return

org endd
	retlw 0x8F
	retlw 0x3B
