.segment "CHR"
	.incbin "background.chr"
	.incbin "sprites.chr"
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
.incbin "palette.pal"
.byte $0F,$18,$28,$38 ; sp0 yellow
.byte $0F,$14,$24,$34 ; sp1 purple
.byte $0F,$1B,$2B,$3B ; sp2 teal
.byte $0F,$12,$22,$32 ; sp3 marine
opening_screen:    .incbin "opening_screen.nam"
select_screen:     .incbin "level_select.nam"
all_levels:        .incbin "levels.bin"

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
	ldx #<opening_screen
	ldy #>opening_screen
	jsr draw_background
	jsr ppu_update
	
	; wait for frame count to rollover or until user presses start to show level select screen
:
	lda gamepad_trigger
	and #PAD_START
	ora count_rollover
	beq :-
	
	; reset rollover
	lda #0
	sta count_rollover
	
	; draw level select screen
	jsr ppu_off
	ldx #<select_screen
	ldy #>select_screen
	jsr draw_background
	jsr ppu_update
	
	lda #0
	sta curr_level
	
	lda #20
	sta delay_long
	lda #4
	sta delay_short
	
NUM_LEVELS = 100
	
@loop_level_select:
	lda gamepad_trigger
	and #PAD_START
	beq :+		
		jsr ppu_off
		lda curr_level
		jsr load_level
		jsr draw_level
		
		; increase delay for player movement
		lda #20
		sta delay_long
		lda #7
		sta delay_short
		
		jmp @play_level ; user has selected a level
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
	ldx #13
	ldy #8
	
	jsr ppu_update_tile
	
	pla
	ldx #14
	jsr ppu_update_tile
		
	jsr ppu_update
	jmp @loop_level_select
	
@play_level:
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

	jsr draw_player
	jsr ppu_update
	jmp @play_level
	