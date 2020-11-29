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
.globalzp temp, count_rollover, nmi_count
.global oam, palette, nmi, ppu_update, ppu_off, ppu_skip, ppu_update_tile

.endif