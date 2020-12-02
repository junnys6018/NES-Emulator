.ifndef GLOBAL_INC
.define GLOBAL_INC

; blockoban.asm
.global main, all_levels

; reset.asm
.global reset

; gamepad.asm
.globalzp gamepad_trigger, gamepad_press, delay_long, delay_short
.global gamepad_poll

; nmi.asm
.globalzp nmi_count, count_rollover, nmt_update_len, scroll_nmt
.global oam, palette, nmi, ppu_update, ppu_off, ppu_skip, ppu_update_tile, ppu_update_metatile, ppu_update_string

; util.asm
.global bin_to_dec

; vars.asm
.globalzp  temp, fill_addr, player_pos, curr_level, buttons_pressed, pause_option

; drawing.asm
.global draw_background, draw_level, draw_player, draw_pause_selector

; level.asm
.global level_data, load_level, move_player

; irq.asm
.global irq

.endif