;
; drawing.asm
;

.include "nes.inc"
.include "global.inc"

.segment "CODE"
; draw_background: used with rendering turned off fills first nametable with data
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

; draw_level: used with rendering turned off, draws the loaded level
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
				lda #TILE_WALL
				jmp @tile_a
			:
			cmp #$20 ; crate tile
			bne :+
				lda #TILE_CRATE
				jmp @tile_a 
			:
			cmp #$30 ; flag
			bne :+
				lda #TILE_FLAG
				jmp @tile_a
			:
			; check bg tiles
			lda temp
			and #$0F
			cmp #$01 ; button
			bne :+
				lda #TILE_BUTTON
				jmp @tile_a
			: ; else, air tile
			lda #TILE_AIR
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


