init
	; hush doggie hush
	banksel WDTCON
	movlw 0b0001_0111
	movwf WDTCON

	; configure PORTC (for display)
	banksel TRISC
	clrf TRISC
	banksel PORTC
	clrf PORTC

	; configure UART
	; use 8-bit 9600 baud
	banksel TRISB
	bsf TRISB, 6
	bsf TRISB, 7
	banksel ANSEL
	bcf ANSELH, 3
	banksel RCSTA
	movlw 0b1000_0000
	movwf RCSTA
	banksel BAUDCTL
	movlw 0b0010_0000
	movwf TXSTA
	movlw 0b0000_1000
	movwf BAUDCTL
	movlw 129
	movwf SPBRG
	clrf SPBRGH

	; turn on interrupts
	banksel INTCON
	bsf INTCON, GIE
	bsf INTCON, PEIE

	; clear timer flags
	banksel 0
	clrf tmrflags

	; set up timer0
	banksel OPTION_REG
	movlw 0b1100_0111 ; use 1:256 prescale
	movwf OPTION_REG
	banksel INTCON
	bsf INTCON, T0IE

	; turn on timer1
	banksel 0
	clrf TMR1L
	clrf TMR1H
	banksel PIE1
	bsf PIE1, TMR1IE
	banksel T1CON
	movlw 0b0000_0001 ; ~ .0000002 sec steps
	movwf T1CON

	; init the display
	banksel 0
	btfss tmrflags, 1 ; we have to wait
	goto $-1
	call lcd_init

	; k display is going
	banksel PIE2
	bsf PIE2, OSFIE

	; check oscillator
	banksel OSCCON
	btfss OSCCON, OSTS ; takes long time to init display, short time to start osc
	call oscfail
	btfsc OSCCON, SCS
	call oscfail

	; print hello
	banksel 0
	movlw 0x80
	call lcd_cmd
	print string.starting1
	movlw 0xC0
	call lcd_cmd
	print string.starting2

	; do checksum of ROM
	bsf PCLATH, 3
	call checkrom
	bcf PCLATH, 3
	btfss STATUS, C
	goto sum_ok
	showmsg string.ecsum
	movlw 0xC0
	call lcd_cmd
	movf cshi, W
	call printhex8
	movf cslo, W
	call printhex8
	movlw 0xCC
	call lcd_cmd
	bsf PCLATH, 3
	call endd
	bcf PCLATH, 3
	call printhex8
	bsf PCLATH, 3
	call endd+1
	bcf PCLATH, 3
	call printhex8
	goto $
sum_ok

	banksel 0 ; all our base belong to bank0
	return
