;
; nmi.asm
;

.include "nes.inc"
.include "global.inc"

; variables used in nmi handle
.segment "ZEROPAGE"
nmi_count:      .res 1 ; is incremented every NMI
nmi_ready:      .res 1 ; 0: not ready to push a frame; 1: push a PPU frame update; 2: disable rendering in next NMI
nmt_update_len: .res 1 ; number of bytes in nmt_update buffer
scroll_nmt:     .res 1 ; nametable select (0-3 = $2000,$2400,$2800,$2C00)

; nmt_update documentation
;
; The main loop can send drawing instructions for the nmi handle to process.
; the instructions are stored in a 256 byte buffer in a bytecode format. Below is a specification of the bytecode
; 
; Byte 0
; 7--- ---0
; BBXX XXXX
; |||| ||||
; ||++-++++- useage depends on bits 6 and 7, see below
; ++-------- opcode of the instruction. 00: update 8x8 tile; 01: update 16x16 metatile;
;                                       10: draw a string; Note: we have room for 1 more opcode if needed
; opcode 00: update 8x8 tile
;
; Byte 0
; 7--- ---0
; BBHH HHHH
;   || ||||
;   ++-++++- High byte of PPU address

; Byte 1
; 7--- ---0
; LLLL LLLL
; |||| ||||
; ++++-++++- Low byte of PPU address

; Byte 2
; 7--- ---0
; DDDD DDDD
; |||| ||||
; ++++-++++- Data to write at given address
;
; opcode 01: update 16x16 metatile
;
; The nes screen is 256x240 pixels, or 16x15 metatiles, this instruction draws a grid aligned metatile onto the screen
; metatiles are stored in the pattern table as 4 consecutive 8x8 tiles, with the metatile starting index aligned every 4 bytes.
; ie. metatiles can be stored at indices $00, $04, $08, ... , $F8, $FC
; 
; Byte 0
; 7--- ---0
; BBII IIII
;   || ||||
;   ++-++++- High bytes of the index into pattern table of the metatile to draw
;
; Byte 1
; 7--- ---0
; YYYY XXXX
; |||| ||||
; |||| ++++- X position of metatile
; ++++------ Y position of metatile
;
; opcode 10: draw a string
;
; Byte 0
; 7--- ---0
; BBLL LLLL
;   || ||||
;   ++-++++- Length of the string to draw
;
; Byte 1
; 7--- ---0
; ..HH HHHH
;   || ||||
;   ++-++++- High byte of PPU address

; Byte 2
; 7--- ---0
; LLLL LLLL
; |||| ||||
; ++++-++++- Low byte of PPU address
;
; Next LLLLLL bytes
; 7--- ---0
; IIII IIII
; |||| ||||
; ++++-++++- character to draw as an index into pattern table

.segment "BSS"
nmt_update:      .res 256 ; nametable update buffer for PPU update
palette:         .res 32  ; palette buffer for PPU update
attribute_table: .res 64  ; local copy of attribute table for nametable 0

.segment "OAM"
oam: .res 256        ; sprite OAM data to be uploaded by DMA

.segment "CODE"
nmi:
	; save registers
	pha
	txa
	pha
	tya
	pha
	
	; increment frame counter
	inc nmi_count
	
	lda nmi_ready
	bne :+ ; nmi_ready == 0 not ready to update PPU
		jmp @ppu_update_end
	:
	cmp #2 ; nmi_ready == 2 disable rendering
	bne :+
		lda #%00000000
		sta PPUMASK
		ldx #0
		stx nmi_ready
		jmp @ppu_update_end
	: ; nmi_ready == 1 push a PPU frame
	; sprite OAM DMA
	lda #0
	sta OAMADDR
	lda #>oam
	sta OAMDMA
	
	; palettes
	lda #%10001000
	sta PPUCTRL ; set horizontal nametable increment
	lda PPUSTATUS ; clear write toggle
	lda #$3F
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $3F00
	
	ldx #0
	:
		lda palette, X
		sta PPUDATA
		inx
		cpx #32
		bcc :-
		
	; nametable update
	ldx #0
	cpx nmt_update_len
	bcs @scroll ; branch if X >= nmt_update_len
	@nmt_update_loop:
		lda nmt_update, X
		and #%11000000
		cmp #%00000000
		bne :+
			lda nmt_update, X
			sta PPUADDR
			inx
			lda nmt_update, X
			sta PPUADDR
			inx
			lda nmt_update, X
			sta PPUDATA
			inx
		:
		cmp #%01000000
		bne :+
			jsr draw_metatile
		:
		cmp #%10000000
		bne :+
			jsr draw_string
		:
		cpx nmt_update_len
		bcc @nmt_update_loop ; branch if X < nmt_update_len
	lda #0
	sta nmt_update_len
	
@scroll:
	lda scroll_nmt
	and #%00000011 ; keep only lowest 2 bits to prevent error
	ora #%10001000
	sta PPUCTRL
	
	lda #$00
	sta PPUSCROLL
	sta PPUSCROLL
	
	; enable rendering
	lda #%00011110
	sta PPUMASK
	; flag PPU update complete
	lda #0
	sta nmi_ready
	
@ppu_update_end:
	; sound engine code goes here
	
@nmi_end:
	; restore registers and return
	pla
	tay
	pla
	tax
	pla
	
	rti

.segment "CODE"

; draw_metatile: draw a 16x16 metatile
draw_metatile:
	lda nmt_update, X
	inx
	asl
	asl
	sta temp
	lda nmt_update, X
	inx
	sta temp+1
	
	and #%11000000
	clc
	rol
	rol
	rol

	ora #$20 ; high 2 bits + $20
	sta PPUADDR
	
	lda temp+1
	and #$0F
	asl
	sta temp+2
	lda temp+1
	and #$F0
	asl
	asl
	ora temp+2
	
	sta PPUADDR
	
	ldy temp
	sty PPUDATA
	iny
	sty PPUDATA
	iny
	sty temp
	
	lda temp+1
	
	and #%11000000
	clc
	rol
	rol
	rol
	
	ora #$20 ; high 2 bits + $20
	sta PPUADDR
	
	lda temp+1
	and #$0F
	asl
	ora #%00100000
	sta temp+2
	lda temp+1
	and #$F0
	asl
	asl
	ora temp+2
	
	sta PPUADDR
	
	ldy temp
	sty PPUDATA
	iny
	sty PPUDATA	
	rts
	
draw_string:
	lda nmt_update, X
	inx
	and #%00111111
	tay
	lda nmt_update, X
	inx
	sta PPUADDR
	lda nmt_update, X
	inx
	sta PPUADDR
	
	:
		lda nmt_update, X
		inx
		sta PPUDATA
		dey
		bne :-
	
	rts
	
;
; drawing utilities
;

.segment "CODE"

; wait until next nmi, turns rendering on (if not already), uploads palette and nametable buffer to PPU
ppu_update:
	lda #1
	sta nmi_ready
	:
		lda nmi_ready
		bne :-
	rts
		
; ppu_skip: waits until next NMI, does not update PPU
ppu_skip:
	lda nmi_count
	:
		cmp nmi_count
		beq :-
	rts

; ppu_off: waits until next NMI, turns rendering off (now safe to write PPU directly via $2007)
ppu_off:
	lda #2
	sta nmi_ready
	:
		lda nmi_ready
		bne :-
	rts
	
; ppu_update_tile: can be used with rendering on, sets the tile at X/Y to tile A next time you call ppu_update
;   Y =  0- 31 nametable $2000
;   Y = 32- 63 nametable $2400
;   Y = 64- 95 nametable $2800
;   Y = 96-127 nametable $2C00
ppu_update_tile:
	pha ; temporarily store A on stack
	txa
	pha ; temporarily store X on stack
	ldx nmt_update_len
	tya
	lsr
	lsr
	lsr
	ora #$20 ; high bits of Y + $20
	sta nmt_update, X
	inx
	tya
	asl
	asl
	asl
	asl
	asl
	sta temp
	pla ; recover X value (but put in A)
	ora temp
	sta nmt_update, X
	inx
	pla ; recover A value (tile)
	sta nmt_update, X
	inx
	stx nmt_update_len
	
	rts

; ppu_update_metatile: used with rendering on. Sets the metatile (16x16 tile) at position X to metatile with index A
ppu_update_metatile:
	ldy nmt_update_len
	ora #%01000000
	
	sta nmt_update, Y
	iny
	
	txa
	sta nmt_update, Y
	iny
	sty nmt_update_len
	rts

; ppu_update_string: used with rendering on. copies a draw string command from address X/Y into the buffer
; set X/Y to the starting address of the command, X low byte, Y high byte	
ppu_update_string:
	stx fill_addr
	sty fill_addr+1
	
	ldy #0
	ldx nmt_update_len
	
	lda (fill_addr), Y
	iny
	sta nmt_update, X
	inx
	
	and #%00111111
	clc
	adc #3 ; length of instruction is 3 bytes longer than length of string
	sta temp
	
	:
		lda (fill_addr), Y
		iny
		sta nmt_update, X
		inx
		cpy temp
		bne :-
	
	stx nmt_update_len
	
	rts
	
.segment "RODATA"
palette_bitmask_lut: .byte %11111100, %11110011, %11001111, %00111111

.segment "ZEROPAGE"
tile_location:    .res 1
palette_quadrant: .res 1
attribute_index:  .res 1
palette_update:   .res 1

.segment "CODE"	
; ppu_update_palette: used with rendering on. Updates 16x16 metatile at location A. with palette specified by lower 2 bits of Y
; preserves X register
ppu_update_palette:
	sta tile_location
	txa
	pha
	lda tile_location
	
	and #$0F
	lsr
	sta attribute_index
	
	lda tile_location
	and #%11100000
	lsr
	lsr
	ora attribute_index
	sta attribute_index ; attribute_index = (A & %11100000) >> 2 | (A & $0F) >> 1
	
	lda tile_location
	and #1
	sta palette_quadrant
	lda tile_location
	and #%00010000
	lsr
	lsr
	lsr
	ora palette_quadrant; A = (A & %00010000) >> 3 | (A $ 1)
	sta palette_quadrant
	
	ldx attribute_index
	lda attribute_table, X
	ldx palette_quadrant
	and palette_bitmask_lut, X
	
	sta palette_update
	tya
	
	ldx palette_quadrant ; load again to set zero flag
	
	:
		beq :+
		asl 
		asl
		dex
		jmp :-
	:
	ora palette_update
	sta palette_update
	ldx attribute_index
	sta attribute_table, X
	
	ldy nmt_update_len
	lda #$23
	sta nmt_update, Y
	iny
	
	lda #$C0
	clc
	adc attribute_index
	sta nmt_update, Y ; palettes start at PPUADDR=$23C0
	iny
	
	lda palette_update
	sta nmt_update, Y
	iny
	
	sty nmt_update_len
	
	pla
	tax ; restore X
	
	rts