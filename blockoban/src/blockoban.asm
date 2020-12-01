.segment "CHR"
    ;.incbin "numbers.chr"
	.incbin "background.chr"
	.incbin "sprites.chr"
.segment "HEADER"
    .byte "NES", $1A, 2, 1 ; 32K PRG, 8K CHR
.segment "VECTORS"
    .word nmi, reset, irq
	
.include "nes.inc"
.include "global.inc"
	
;
; irq routine
;

.segment "CODE"
irq:
	rti
	
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

.segment "ZEROPAGE"
curr_level: .res 1
pos_x:      .res 1
pos_y:      .res 1

.segment "LEVEL"
level_data: .res 256

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
		

.segment "ZEROPAGE"
fill_addr: .res 2 ; used as a temp variable for indirect addressing

.segment "CODE"
; set X/Y to the starting address of the nametable, X low byte, Y high byte
draw_background:
	stx fill_addr
	sty fill_addr+1

	lda PPUSTATUS ; reset latch
	lda #$20
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $2000
	
	; fill $400 bytes
	ldx #0
	:
		ldy #0
		:
			lda (fill_addr), Y
			sta PPUDATA
			iny
			bne :-
		inc fill_addr+1 ; load from next page
		inx
		cpx #$04
		bne :--

	rts
	
; set A to the level to load
load_level:
	clc
	adc #>all_levels
	sta fill_addr+1
	
	lda #<all_levels
	sta fill_addr
	
	ldy #0
	:
		lda (fill_addr), Y
		sta level_data, Y
		iny
		bne :-
		
	; place starting player position at the flag
	lda level_data+$F0 ; flag positon at offset $F0
	and #$0F
	sta pos_x
	
	lda level_data+$F0
	lsr
	lsr
	lsr
	lsr
	sta pos_y
	
	rts
	
; draw the loaded level
draw_level:	
	lda PPUSTATUS ; reset latch
	lda #$20
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $2000

	; loop y from [0..30), x from [0..32)
	ldy #0
	@outer:
		ldx #0
		@inner:
			tya
			pha ; save y
			
			lsr ; calculate index into level data, index = ((Y >> 1) << 4) | (X >> 1)
			asl
			asl
			asl
			asl
			sta temp
			txa
			lsr
			ora temp
			tay
			lda level_data, Y
			sta temp
			pla
			tay ; retrieve y
			lda temp
			and #$F0
			cmp #$10 ; wall tile
			bne :+
				lda #$30
				jmp @tile_a
			:
			cmp #$20 ; crate tile
			bne :+
				lda #$34
				jmp @tile_a 
			:
			cmp #$30 ; flag
			bne :+
				lda #$38
				jmp @tile_a
			:
			; check bg tiles
			lda temp
			and #$0F
			cmp #$01 ; button
			bne :+
				lda #$3C
				jmp @tile_a
			: ; else, air tile
			lda #$FF
			jmp @load_a
		@tile_a:
			sta temp
			txa
			and #$01
			beq :+
				inc temp
			:
			tya
			and #$01
			beq :+
				inc temp
				inc temp
			:
			lda temp
		@load_a:
			sta PPUDATA
			
			inx
			cpx #32
			bne @inner
		iny
		cpy #30
		bne @outer
		
	; set all attributes to 1
	lda #%01010101
	ldx #64 ; 64 bytes
	:
		sta PPUDATA
		dex
		bne :-
		
	rts
	
draw_player:
	lda pos_y
	asl
	asl
	asl
	asl
	sec
	sbc #1 ; subtract 1 because sprite rendering is delayed by one scanline
	
	sta oam+(0*4)+0
	sta oam+(1*4)+0
	
	clc
	adc #8
	sta oam+(2*4)+0
	sta oam+(3*4)+0
	
	lda pos_x
	asl
	asl
	asl
	asl

	sta oam+(0*4)+3
	sta oam+(2*4)+3
	
	adc #8
	sta oam+(1*4)+3
	sta oam+(3*4)+3
	
	
	lda #0
	sta oam+(0*4)+2
	sta oam+(1*4)+2
	sta oam+(2*4)+2
	sta oam+(3*4)+2
	
	sta oam+(0*4)+1
	
	lda #1
	sta oam+(1*4)+1
	
	lda #2
	sta oam+(2*4)+1
	
	lda #3
	sta oam+(3*4)+1
	
	rts

.segment "ZEROPAGE"
level_increment:       .res 1
player_new_position:   .res 1
bg_tile_replace:       .res 1

.segment "RODATA"
direction_lut: .byte %11110000, $10, $FF, 1 ; -32, 32, -1, 1

; move_player: attempts to move the player in a given direction 
; modifying the level and rendering any changes if the player successfully moved
; set A to the direction the player moved
; 0: no move; 1: up; 2: down; 3: left; 4: right
.segment "CODE"
move_player:
	cmp #0
	bne :+
		rts ; no move, immediately return
	:
	tax
	lda direction_lut-1, X
	sta level_increment
	
	lda pos_y
	asl
	asl
	asl
	asl
	ora pos_x ; A = players position as an index into the level 

	clc
	adc level_increment ; A = index player wants to move to
	sta temp
	sta player_new_position
	
	tax
	lda level_data, X
	and #$F0
	
	cmp #$10
	bne :+ ; player wants to move into a wall, return
		rts
	:
	cmp #$20
	bne @update_position ; branch if player is not moving into a crate
	@begin_crate_test: 
	; if player wants to move into a crate, we need to check if there is an air space for the crate to move
		lda temp
		clc
		adc level_increment
		sta temp ; temp = index adjacent to the crate
		tax
		lda level_data, X
		and #$F0
		
		cmp #$00
		bne @done_air ; next tile is air, so we can move
		
			; set air tile to a crate
			lda level_data, X
			ora #$20
			sta level_data, X
			
			; set the original crate tile into an air tile
			ldx player_new_position
			lda level_data, X
			and #$0F
			sta level_data, X
			sta bg_tile_replace ; remember the background tile for drawing
			
			; send draw commands
			
			; draw crate
			ldx temp
			lda #($34 >> 2)
			jsr ppu_update_block
			
			ldx player_new_position
			lda bg_tile_replace
			cmp #1
			bne :+ 
				lda #($3C >> 2) ; draw button
				jmp :++
			: 
				lda #($FC >> 2) ; draw air
			:
			
			jsr ppu_update_block
		
			jmp @update_position ; player successfully moved
		@done_air:
		cmp #$20
		bne :+ ; next tile is a crate, so we repeat the test
			jmp @begin_crate_test
		:
		rts ; else, next tile is a wall or flag, we cant move
		
@update_position: ; if we got here, it means the player was able to move, so we update its positon
	lda player_new_position
	and #$0F
	sta pos_x
	
	lda player_new_position
	lsr
	lsr
	lsr
	lsr
	sta pos_y

	rts