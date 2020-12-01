;
; vars.asm: defines global variables
;

.include "nes.inc"
.include "global.inc"

.segment "ZEROPAGE"
temp:           .res 3 ; temp variables
fill_addr:      .res 2 ; used as a temp variable for indirect addressing
pos_x:          .res 1
pos_y:          .res 1
curr_level:     .res 1 ; level we are currently playing
