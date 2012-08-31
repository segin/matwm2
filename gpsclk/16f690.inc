#ifndef __PIC16F690_INC__
#define __PIC16F690_INC__

#include "banksel.inc"

#define W 0
#define F 1

; indirect adressing
#define INDF   0x00
#define PCL    0x02
#define FSR    0x04
#define PCLATH 0x0A

; ports
#define PORTA 0x05
#define PORTB 0x06
#define PORTC 0x07
#define TRISA 0x85
#define TRISB 0x86
#define TRISC 0X87
#define ANSEL 0x11E
#define ANSELH 0x11F

; status
#define STATUS 0x83
#define C   0
#define DC  1
#define Z   2
#define PD  3
#define TO  4
#define RP0 5
#define RP1 6
#define IRP 7

; watchdog
#define WDTCON 0x97

; option_reg
#define OPTION_REG 0x81

; interrupt stuff
#define INTCON 0x0B
#define PIR1 0x0C
#define PIR2 0x0D
#define PIE1 0x8C
#define PIE2 0x8D

; intcon
#define RABIF 0
#define INTF  1
#define T0IF  2
#define RABIE 3
#define INTE  4
#define T0IE  5
#define PEIE  6
#define GIE   7
; pie1
#define TMR1IE 0
#define TMR2IE 1
#define CCP1IE 2
#define SSPIE  3
#define TXIE   4
#define RCIE   5
#define ADIE   6
; pie2
#define EEIE  4
#define C1IE  5
#define C2IE  6
#define OSFIE 7
; pir1
#define TMR1IF 0
#define TMR2IF 1
#define CCP1IF 2
#define SSPIF  3
#define TXIF   4
#define RCIF   5
#define ADIF   6
; pir2
#define EEIF  4
#define C1IF  5
#define C2IF  6
#define OSFIF 7

; osccon
#define OSCCON 0x8F
#define OSTS 3
#define HTS  2
#define LTS  1
#define SCS  0

; timer0
#define TMR0 0x01

; timer1
#define TMR1L 0x0E
#define TMR1H 0x0F
#define T1CON 0x10
#define TMR1ON  0
#define TMR1CS  1
#define T1SYNC  2
#define T1OSCEN 3
#define T1CKPS0 4
#define T1CKPS1 5
#define TMR1GE  6
#define T1GINV  7

; eusart
#define RCSTA   0x18
#define TXSTA   0x98
#define SPBRG   0x99
#define SPBRGH  0x9A
#define BAUDCTL 0x9B
#define TXREG   0x19
#define RCREG   0x1A
; rcsta
#define RX9D  0
#define OERR  1
#define FERR  2
#define ADDEN 3
#define CREN  4
#define SREN  5
#define RX9   6
#define SPEN  7
; txsta
#define TX9D 0
#define TRMT 1
#define BRGH 2
#define SENB 3
#define SYNC 4
#define TXEN 5
#define TX9  6
#define CSRC 7
; baudctl
#define ABDEN  0
#define WUE    1
#define BRG16  3
#define SCKP   4
#define RCIDL  6
#define ABDOVF 7 

; eeprom etc
#define EEDAT  0x10C
#define EEADR  0x10D
#define EEDATH 0x10E
#define EEADRH 0x10F
#define EECON1 0x18C
#define EECON2 0x18D
#define RD     0
#define WR     1
#define WREN   2
#define WRERR  3
#define EEPGD  7

#endif ; __PIC16F690_INC__
