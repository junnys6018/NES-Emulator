;
; nmi routine
;

.include "nes.inc"
.include "global.inc"

; variables used in nmi handle
.segment "ZEROPAGE"
nmi_count:      .res 1 ; is incremented every NMI
count_rollover: .res 1 ; set to 1 whenever nmi_count overflows
nmi_ready:      .res 1 ; 0: not ready to push a frame; 1: push a PPU frame update; 2: disable rendering in next NMI
nmt_update_len: .res 1 ; number of bytes in nmt_update buffer
temp:           .res 1 ; temp variable

; buffering for nmi 
.segment "BSS"
nmt_update: .res 256 ; nametable update buffer for PPU update
palette:    .res 32  ; palette buffer for PPU update

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
	bne :+
		lda #1
		sta count_rollover
	:
	
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
		sta PPUADDR
		inx
		lda nmt_update, X
		sta PPUADDR
		inx
		lda nmt_update, X
		sta PPUDATA
		inx
		cpx nmt_update_len
		bcc @nmt_update_loop ; branch if X < nmt_update_len
	lda #0
	sta nmt_update_len
	
@scroll:
	; fix scroll to top left corner of first nt
	lda #%10001000
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
	jsr gamepad_poll
	
@nmi_end:
	; restore registers and return
	pla
	tay
	pla
	tax
	pla
	
	rti
	
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
	sta temp ;
	pha ;
	tya ;
	pha ;
	txa ;
	pha ;
	lda temp ;
	
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
	
	pla ;
	tax ;
	pla ;
	tay ;
	pla ;
	
	rts