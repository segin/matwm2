	alloc uart_tmp

rcv_enable
	bsf RCSTA, CREN
	btfsc RCSTA, FERR
	movf RCREG, W
	btfsc RCSTA, FERR
	goto $-2
	return

xmit
	banksel TXSTA
	btfss TXSTA, TRMT
	goto $-1
	banksel 0
	movwf TXREG
	return

recv
	movwf uart_tmp
	movlw recvdata
	movwf FSR
.loop
	btfsc RCSTA, OERR
	goto eovfl
	btfsc RCSTA, FERR
	goto eframe
	bcf PIR1, RCIF
	btfss PIR1, RCIF
	goto .loop
	movf RCREG, W
	movwf INDF
	incf FSR
	decfsz uart_tmp
	goto .loop
	return

eframe
	showmsg string.eframe
	movf RCREG, W ; clear framing error
	goto recv.loop

eovfl
	showmsg string.eovfl
	bcf RCSTA, CREN ; clears overflow
	bsf RCSTA, CREN
	goto recv.loop

