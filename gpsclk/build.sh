#!/bin/sh
prog=gpsclk
make mksum
if matpic $prog.asm > $prog.hex; then
	cat $prog.hex | sed "s/:00000001FF/`cat $prog.hex | ./mksum`/" > $prog.hex
	echo ":00000001FF" >> $prog.hex
	pk2cmd -PPIC16F690 -R -T -M -F$prog.hex
fi

