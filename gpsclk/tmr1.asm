t1_wait
	btfss tmrflags, 1
	goto $-1
	return

t1_set80
	bcf T1CON, TMR1ON
	movlw 0xFF
	movwf TMR1H
	movlw 0x80 ; this is approx 50µS (if i don't mistake)
	movlw TMR1L
	bcf tmrflags, 1
	bsf T1CON, TMR1ON
	return

t1_set100
	bcf T1CON, TMR1ON
	movlw 0xFE
	movwf TMR1H
	movlw 0xFF ; this is approx 100µS (if i don't mistake)
	movlw TMR1L
	bcf tmrflags, 1
	bsf T1CON, TMR1ON
	return

t1_reset
	bcf T1CON, TMR1ON
	clrf TMR1H
	clrf TMR1L
	bcf tmrflags, 1
	bsf T1CON, TMR1ON
	return
