macro enum s
 xdefine __enum [s]
 rep @0
  @[1 + @@]: equ __enum
  xdefine __enum [__enum + 1]
 endrep
endm
