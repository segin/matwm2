init
	; hush doggie hush
	banksel WDTCON
	movlw 0b0000_1011
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
	movlw 0b1001_0000
	movwf RCSTA
	banksel BAUDCTL
	movlw 0b0010_0000
	movwf TXSTA
	movlw 0b0000_1000
	movwf BAUDCTL
	movlw 129
	movwf SPBRG
	clrf SPBRGH

	banksel 0
	call longdelay
	call longdelay
	call lcd_init

	; turn on interrupts
	banksel 0x80
	bsf INTCON, GIE
	bsf INTCON, PEIE
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

	banksel 0 ; all our base belong to bank0
	return

