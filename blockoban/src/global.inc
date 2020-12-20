.ifndef GLOBAL_INC
.define GLOBAL_INC

; blockoban.asm
.global main, all_levels, end_screen

; reset.asm
.global reset

; gamepad.asm
.globalzp gamepad_trigger, gamepad_press, delay_long, delay_short
.global gamepad_poll

; nmi.asm
.globalzp nmi_count, nmt_update_len, scroll_nmt
.global oam, palette, attribute_table
.global nmi, ppu_update, ppu_off, ppu_skip, ppu_update_tile, ppu_update_metatile, ppu_update_string, ppu_update_palette

; util.asm
.global bin_to_dec, wait_for_enter

; vars.asm
.globalzp  temp, fill_addr, player_pos, curr_level, buttons_pressed, pause_option

; drawing.asm
.global draw_background, draw_level, draw_metatile_sprite, set_metatile_sprite_palette, draw_pause_selector, animate_flag

; level.asm
.global level_data, load_level, move_player, set_flag_color

; irq.asm
.global irq

; end_screen.asm
.global setup_end_screen

; sound_data.asm
.global sounds

.endif