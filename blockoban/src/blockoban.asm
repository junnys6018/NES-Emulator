.segment "CHR"
	.incbin "res/background.chr"
	.incbin "res/sprites.chr"
.segment "HEADER"
    .byte "NES", $1A, 2, 1 ; 32K PRG, 8K CHR
.segment "VECTORS"
    .word nmi, reset, irq
	
.include "nes.inc"
.include "global.inc"
	
;
; main
;

; include read only game data (nametable, palettes, levels)
.segment "RODATA"
example_palette:
.incbin "res/palette.pal"
sprite_palette:
.byte $0F,$18,$28,$38 ; sp0 yellow
.byte $0F,$14,$24,$34 ; sp1 purple
.byte $0F,$1B,$2B,$3B ; sp2 teal
.byte $0F,$19,$2A,$27 ; sp3 marine
opening_screen:    .incbin "res/opening_screen.nam"
end_screen:        .incbin "res/end_screen.nam"
select_screen:     .incbin "res/level_select.nam"
all_levels:        .incbin "bin/levels.bin"

; draw string commands
draw_str_pause:   .byte %10000110, $29, $4D, "PAUSED"
draw_str_resume:  .byte %10000110, $29, $8D, "RESUME"
draw_str_restart: .byte %10000111, $29, $AD, "RESTART"
draw_str_exit:    .byte %10000100, $29, $CD, "EXIT"


.segment "CODE"
main:
	; load palette
	ldx #0
	:
		lda example_palette, X
		sta palette, X
		inx
		cpx #32
		bcc :-
		
	; draw background
	addr_jsr opening_screen, draw_background
	
	; clear second nametable
	lda #$28
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $2800
	
	; fill 960 bytes
	lda #$FF
	ldx #0
	:
		ldy #0
		:
			sta PPUDATA
			iny
			cpy #32
			bne :-
		inx
		cpx #30
		bne :--
		
	; draw pause screen
	addr_jsr draw_str_pause, ppu_update_string
	addr_jsr draw_str_resume, ppu_update_string
	addr_jsr draw_str_restart, ppu_update_string
	addr_jsr draw_str_exit, ppu_update_string
	
	jsr ppu_update
	
	; wait for nmi_count to reach $FF or until user presses start to show level select screen
	jsr wait_for_enter
	
	; draw level select screen
	jsr ppu_off
	addr_jsr select_screen, draw_background
	jsr ppu_update
	
	lda #0
	sta curr_level
	
	lda #20
	sta delay_long
	lda #4
	sta delay_short

;
; various game loops
;
	
loop_level_select:
	jsr gamepad_poll
	lda gamepad_trigger
	and #PAD_START
	beq :+		
		jsr ppu_off
		lda curr_level
		jsr load_level
		jsr draw_level
		
		jmp loop_play_level ; user has selected a level
	:
	lda gamepad_press
	and #PAD_U
	beq :+
		lda curr_level
		clc
		adc #5
		sta curr_level
	:
	lda gamepad_press
	and #PAD_D
	beq :++
		lda curr_level
		cmp #5
		bcc :+
		sec
		sbc #5
		sta curr_level
		jmp :++
		:
		lda #0
		sta curr_level
	:
	lda gamepad_press
	and #PAD_L
	beq :+
		lda curr_level
		beq :+
		dec curr_level
	:
	lda gamepad_press
	and #PAD_R
	beq :+
		inc curr_level
	:
	; check for overflow
	lda curr_level
	cmp #NUM_LEVELS
	bcc :+
		lda #NUM_LEVELS-1
		sta curr_level
	:
	
	; update screen
	lda curr_level
	jsr bin_to_dec
	
	; push y to stack
	tya
	pha
	
	txa 
	ldx #15
	ldy #14
	
	jsr ppu_update_tile
	
	pla
	ldx #16
	jsr ppu_update_tile
		
	jsr ppu_update
	jmp loop_level_select
	
loop_play_level:
	jsr gamepad_poll
	lda gamepad_trigger
	and #PAD_START
	beq :++ ; pause the game
		; set scroll to second nametable (pause screen)
		lda #2
		sta scroll_nmt
		
		; clear player sprite and flag
		lda #$FF
			ldx #0
			:
				sta oam, X
				inx
				cpx #$20
				bne :-
		
		lda #0
		sta pause_option

		jmp loop_pause_level
	:
	lda gamepad_press
	and #PAD_U
	beq :+
		lda #1
		jmp @done
	:
	lda gamepad_press
	and #PAD_D
	beq :+
		lda #2
		jmp @done
	:
	lda gamepad_press
	and #PAD_L
	beq :+
		lda #3
		jmp @done
	:
	lda gamepad_press
	and #PAD_R
	beq :+
		lda #4
		jmp @done
	:
	lda #0
@done:
	jsr move_player 
	
	lda buttons_pressed
	cmp level_data+$F1 ; compare with number of buttons in the level
	bne @end_next_level
		lda level_data+$F0
		cmp player_pos
		bne @end_next_level
			; clear any draw commands move_player might have sent
			lda #0
			sta nmt_update_len
			
			; load next level
			jsr ppu_off
			inc curr_level
			lda curr_level
			cmp #NUM_LEVELS
			bne :++
				; last level played, go to end screen
				
				; clear sprites
				lda #$FF
				ldx #0
				:
					sta oam, X
					inx
					cpx #$20
					bne :-
				jmp setup_end_screen
			:
			
			jsr load_level
			jsr draw_level
			jmp loop_play_level
	@end_next_level:

	lda player_pos
	ldx #0
	ldy #OAM_PLAYER
	jsr draw_metatile_sprite
	
	jsr animate_flag
	jsr set_flag_color
	
	jsr ppu_update
	jmp loop_play_level

.segment "CODE"	
loop_pause_level:
	jsr gamepad_poll
	lda gamepad_trigger
	and #PAD_START
	beq @end
		; reset nt
		lda #0
		sta scroll_nmt
			
		; clear pause selector sprite
		lda #$FF
		sta oam+OAM_PAUSE_SELECT+0
		sta oam+OAM_PAUSE_SELECT+1
		sta oam+OAM_PAUSE_SELECT+2
		sta oam+OAM_PAUSE_SELECT+3
		
		lda pause_option
		cmp #2 
		bne :++ ; exit
			jsr ppu_off
			
			lda #0
			sta nmi_count
			
			lda #$FF
			ldx #0
			:
				sta oam, X
				inx
				cpx #$10
				bne :-
			jmp main
		:
		; redraw the flag
		lda level_data+$F0
		ldx #$10
		ldy #OAM_FLAG
		jsr draw_metatile_sprite
		
		jsr set_flag_color
		
		lda pause_option
		cmp #0
		bne :+ ; resume				
			jmp loop_play_level
		:
		cmp #1 
		bne :+ ; restart
			jsr ppu_off
			lda curr_level
			jsr load_level
			jsr draw_level

			jmp loop_play_level ; user has selected a level
		:
	@end:
	lda gamepad_trigger
	and #PAD_U
	beq :+
		lda pause_option
		beq :+
		dec pause_option
	:
	lda gamepad_trigger
	and #PAD_D
	beq :+
		lda pause_option
		cmp #2
		beq :+
		inc pause_option
	:
	
	jsr draw_pause_selector
	
	jsr ppu_update
	jmp loop_pause_level