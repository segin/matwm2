t0_wait
	btfss tmrflags, 0
	goto $-1
	return

t0_set50
	movlw 0xFF ; approx 50µS to overflow
	movwf TMR0
	bcf tmrflags, 0
	return

t0_set100
	movlw 0xFE ; approx 100µS to overflow
	movwf TMR0
	bcf tmrflags, 0
	return

t0_reset
	clrf TMR0
	bcf tmrflags, 0
	return
