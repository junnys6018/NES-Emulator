;
; vars.asm: defines global variables
;

.include "nes.inc"
.include "global.inc"

.segment "ZEROPAGE"
temp:             .res 3 ; temp variables
fill_addr:        .res 2 ; used as a temp variable for indirect addressing
player_pos:       .res 1 ; as an index into the level
curr_level:       .res 1 ; level we are currently playing
buttons_pressed:  .res 1
pause_option:     .res 1
