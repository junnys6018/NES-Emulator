;
; reset.asm
;

.include "nes.inc"
.include "global.inc"

.segment "CODE"
reset:
	sei       ; disable interrupts
	lda #0
	sta PPUCTRL ; disable NMI
	sta PPUMASK ; disable rendering
	sta APUCTRL ; disable APU sound
	sta DMCCTRL ; disable DMC IRQ
	lda #$40
	sta APUFC ; disable APU IRQ
	cld       ; disable decimal mode
	ldx #$FF
	txs       ; initialize stack
	
	; wait for first vblank
	bit PPUSTATUS
	:
		bit PPUSTATUS
		bpl :-
		
	; clear all RAM to 0 (by clearing page 0 to 7)
	lda #0
	ldx #0
	:
		sta $0000, X
		sta $0100, X
		sta $0200, X
		sta $0300, X
		sta $0400, X
		sta $0500, X
		sta $0600, X
		sta $0700, X
		inx
		bne :-
		
	; clear oam	
	lda #$FF
	ldx #0
	:
		sta oam, X
		inx
		bne :-
		
	; wait for second vblank
	:
		bit PPUSTATUS
		bpl :-
	
	; after 2 vblanks, nes is initialized	
	; enable the NMI for graphical updates, and jump to our main program
	lda #%10001000
	sta PPUCTRL
	jmp main