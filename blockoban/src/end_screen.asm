;
; end_screen.asm: end screen animation
;

.include "nes.inc"
.include "global.inc"

.segment "BSS"
draw_counter: .res 1

.segment "ZEROPAGE"
nmt_indices:  .res 4
coords:       .res 2

.segment "CODE"
setup_end_screen:
	lda #0 
	sta draw_counter
	
	; clear the screen, and fill attribute table
	lda PPUSTATUS ; reset latch
	lda #$20
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $2000
	
	; clear nametable
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
		
	; fill attribute table
	ldx #0
	:
		lda end_screen+960, X
		sta PPUDATA
		inx
		cpx #64
		bne :-
	
	jsr ppu_update
	
	lda #<end_screen
	sta fill_addr
	lda #>end_screen
	sta fill_addr+1

; this drawing routine could probably be optimised quite a bit, but it doesnt matter as animating the end screen is not performance critical
loop_draw_end_screen:
	; preload nametable indices 
	lda draw_counter
	and #$0F
	asl
	
	sta temp
	lda draw_counter
	and #$F0
	asl
	asl
	
	ora temp
	
	tay
	lda (fill_addr), Y
	sta nmt_indices
	
	iny
	lda (fill_addr), Y
	sta nmt_indices+1
	
	tya
	clc
	adc #31 ; next row
	tay
	lda (fill_addr), Y
	sta nmt_indices+2
	
	iny
	lda (fill_addr), Y
	sta nmt_indices+3
	
	; calculate coordinates
	lda draw_counter
	and #$0F
	asl
	tax
	sta coords
	
	lda draw_counter
	and #$F0
	lsr
	lsr
	lsr
	tay
	sta coords+1
	
	lda nmt_indices
	jsr ppu_update_tile
	
	ldx coords
	inx
	ldy coords+1
	lda nmt_indices+1
	jsr ppu_update_tile
	
	ldx coords
	ldy coords+1
	iny
	lda nmt_indices+2
	jsr ppu_update_tile
	
	ldx coords
	inx
	ldy coords+1
	iny
	lda nmt_indices+3
	jsr ppu_update_tile
	
	jsr ppu_update
	
	inc draw_counter
	lda draw_counter
	cmp #240 ; number of metatiles on the screen
	bne :+		
		lda #0
		sta nmi_count
		
		jsr wait_for_enter
			
		lda #0
		sta nmi_count
		
		jsr ppu_off
		jmp main
	:
	and #%00111111
	bne :+
		; start indexing nametable data from next page
		inc fill_addr+1
	:
	
	jmp loop_draw_end_screen