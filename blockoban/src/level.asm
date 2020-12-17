;
; level.asm: aka game logic
;

.include "nes.inc"
.include "global.inc"

; level data documentation
; each level is 256 bytes, a level is made up of a 16x15 grid of tiles. Each tile takes up one byte. 
; Therefore the level can be stored in a contiguous array of 240 bytes. Each tile contains a background
; and foreground element. each byte of the level is formatted as follows.
;
; 7--- ---0
; FFFF BBBB
; |||| ||||
; |||| ++++- Background tile. 0: nothing; 1: button
; ++++------ Foreground tile. 0: nothing; 1: wall; 2: crate; 3: flag
;
; a position in the level can be specified as an 8 bit index into the level array
; The last 16 bytes is used to provide metadata about the level 
;
; Offset | Meaning
; -------+-------------------------------
; 0      | index of the flag
; 1      | number of buttons in the level

.segment "LEVEL"
level_data: .res 256

.segment "CODE"

; load_level: loads a level from rom into memory
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
	sta player_pos
	
	; reset number of buttons pressed
	lda #0
	sta buttons_pressed
	
	rts
	
; move_player: attempts to move the player in a given direction,
; modifying the level and rendering any changes if the player successfully moved
; set A to the direction the player moved
; 0: no move; 1: up; 2: down; 3: left; 4: right	
	
.segment "ZEROPAGE"
level_increment:       .res 1
player_new_position:   .res 1
bg_tile_replace:       .res 1

.segment "RODATA"
direction_lut: .byte %11110000, $10, $FF, 1 ; -32, 32, -1, 1

.segment "CODE"
move_player:
	cmp #0
	bne :+
		rts ; no move, immediately return
	:
	tax
	lda direction_lut-1, X
	sta level_increment
	
	lda player_pos

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
		
			lda level_data, X
			cmp #1
			bne :+
				; check if crate was pushed onto button
				inc buttons_pressed
				
				; change the color of the crate to indicate it is on a button
				txa
				ldy #1
				jsr ppu_update_palette
				lda level_data, X
			:
			
			; set air tile to a crate
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
			lda #(TILE_CRATE >> 2)
			jsr ppu_update_metatile
			
			lda bg_tile_replace
			cmp #1
			bne :+ 
				; player pushed crate off a button, restore palette
				ldy #0
				lda player_new_position
				jsr ppu_update_palette
				
				lda #(TILE_BUTTON >> 2) ; draw button
				dec buttons_pressed
				
				jmp :++
			: 
				lda #(TILE_AIR >> 2) ; draw air
			:
			
			ldx player_new_position
			jsr ppu_update_metatile
		
			jmp @update_position ; player successfully moved
		@done_air:
		cmp #$20
		bne :+ ; next tile is a crate, so we repeat the test
			jmp @begin_crate_test
		:
		rts ; else, next tile is a wall or flag, we cant move
		
@update_position: ; if we got here, it means the player was able to move, so we update its positon
	lda player_new_position
	sta player_pos

	rts