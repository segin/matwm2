; lcd driver
;
; needs lcd_tmp, dc0 and dc1 defined as registers than can be used
; use:
;   wire d4-d7 to lower 4 pints of port
;     tie d0-d2 to gnd and d3 high
;   wait a little
;   call lcd_init
;   call lcd_char to place character (from W)
;   call lcd_cmd with command described in datasheet
;     0x80-0x8F place cursor at char on first line
;     0xC0-0xCF place cursor at char on second line
;     0x01 clear screen
;     0x02 return home
;     0x08 turn screen off
;       to turn things on we can or with this:
;         0x04 screen on
;         0x02 cursor
;         0x01 blinking cursor -- independent of other cursor

lcd_tmp  equ tmp0
lcd_port equ PORTC
lcd_clk  equ 0x40 ; clock bit
lcd_rs   equ 0x10 ; register select bit

lcd_init ; can also be used for reset
	movlw 0x38 ; this is to force 8bit mode (seamlessly)
	call lcd_cmd
	movlw 0x83
	call lcd_cmd
	movlw 0x33
	call lcd_cmd ; so we can have aligned nybble next
	movlw 0x02 | lcd_clk ; switch to 4-bit mode
	call lcd_send
	movlw 0x28 ; 4 wire mode, normal font, 2 lines
	call lcd_cmd
	movlw 0x0C ; turn display on (bit 1 and 0 for cursor and blinking cursor resp.)
	call lcd_cmd
	movlw 0x06 ; left to right, no shift
	call lcd_cmd
	movlw 0x02 ; return home
	call lcd_cmd
	call t0_reset ; return home may take its sweet time
	return

lcd_cmd
	call t0_wait
	movwf lcd_tmp
	swapf lcd_tmp, W
	andlw 0x0F
	iorlw lcd_clk
	call lcd_send
	movf lcd_tmp, W
	andlw 0x0F
	iorlw lcd_clk
	call lcd_send
	call t0_set100
	return

lcd_char
	call t0_wait
	movwf lcd_tmp
	swapf lcd_tmp, W
	andlw 0x0F
	iorlw lcd_clk | lcd_rs ; clock and RS
	call lcd_send
	movf lcd_tmp, W
	andlw 0x0F
	iorlw lcd_clk | lcd_rs ; clock and RS
	call lcd_send
	call t0_set50
	return

lcd_send
	movwf lcd_port
	andlw ~lcd_clk
	movwf lcd_port
	return
