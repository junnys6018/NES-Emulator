.ifndef GLOBAL_INC
.define GLOBAL_INC

; blockoban.asm
.global main

; reset.asm
.global reset

; gamepad.asm
.globalzp gamepad_trigger, gamepad_press, delay_long, delay_short
.global gamepad_poll

; nmi.asm
.globalzp temp, count_rollover
.global oam, palette, nmi, ppu_update, ppu_off, ppu_skip, ppu_update_tile, ppu_update_block

; util.asm
.global bin_to_dec

.endif