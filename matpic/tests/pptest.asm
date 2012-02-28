macro enum s
 xdefine __enum [s]
 rep @0
  @<1 + @@>: equ __enum
  xdefine __enum [__enum + 1]
 endrep
endm
enum 4, four, five, six, seven
enum 1, one, two, three
enum 8, eight, nine, ten
