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
	clrf rstate

	; turn on interrupts
	banksel INTCON
	bsf INTCON, GIE
	bsf INTCON, PEIE

	; turn on timer1
	banksel 0
	bcf tmrflags, 0
	clrf TMR1L
	clrf TMR1H
	banksel PIE1
	bsf PIE1, TMR1IE
	banksel T1CON
	movlw 0b0000_0001 ; ~ .0000002 sec steps
	movwf T1CON

	; init the display
	banksel 0
	btfss tmrflags, 0 ; we have to wait
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

	banksel 0 ; all our base belong to bank0
	return

