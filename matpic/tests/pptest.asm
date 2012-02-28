macro enum s
 xdefine __enum s
 rep @0
  @<1 + @@>: equ __enum
  xdefine __enum [s + 1]
 endrep
endm
enum 4, five, six, seven
