.segment "CHR"
    ;.incbin "numbers.chr"
	.incbin "background.chr"
	.incbin "sprites.chr"
.segment "HEADER"
    .byte "NES", $1A, 2, 1 ; 32K PRG, 8K CHR
.segment "VECTORS"
    .word nmi, reset, irq
	
; Define PPU, APU and IO registers

PPUCTRL        = $2000
PPUMASK        = $2001
PPUSTATUS      = $2002
OAMADDR        = $2003
OAMDATA        = $2004
PPUSCROLL      = $2005
PPUADDR        = $2006
PPUDATA        = $2007
OAMDMA         = $4014

DMCCTRL        = $4010

APUCTRL        = $4015
APUSTATUS      = $4015
APUFC          = $4017 ; APU frame counter

JOY1           = $4016
	

;
; reset routine
;
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
		
	; wait for second vblank
	:
		bit PPUSTATUS
		bpl :-
	
	; after 2 vblanks, nes is initialized
	
	; clear nametable
	lda PPUSTATUS ; clear write toggle
	lda #$20
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $2000
	
	ldy #$0F
	:
		ldx #$FF
		:
			sta PPUDATA
			dex
			bne :-
		dey
		bne :--
	
	; enable the NMI for graphical updates, and jump to our main program
	lda #%10001000
	sta PPUCTRL
	jmp main
	
;
; nmi routine
;

; variables used in nmi handle
.segment "ZEROPAGE"
nmi_count:      .res 1 ; is incremented every NMI
count_rollover: .res 1 ; set to 1 whenever nmi_count overflows
nmi_ready:      .res 1 ; 0: not ready to push a frame; 1: push a PPU frame update; 2: disable rendering in next NMI
nmt_update_len: .res 1 ; number of bytes in nmt_update buffer
temp:           .res 1 ; temp variable

; buffering for nmi 
.segment "BSS"
nmt_update: .res 256 ; nametable update entry buffer for PPU update
palette:    .res 32  ; palette buffer for PPU update

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
	
	; palettes
	lda #%10001000
	sta PPUCTRL ; set horizontal nametable increment
	lda PPUSTATUS ; clear write toggle
	lda #$3F
	sta PPUADDR
	lda #$00
	sta PPUADDR ; set PPU address to $3F00
	
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
		bcc @nmt_update_loop ; branch is X < nmt_update_len
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
	lda #%00001010
	sta PPUMASK
	; flag PPU update complete
	ldx #0
	stx nmi_ready
	
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
	
; ppu_address_tile: use with rendering off, sets memory address to tile at X/Y, ready for a $2007 write
;   Y =  0- 31 nametable $2000
;   Y = 32- 63 nametable $2400
;   Y = 64- 95 nametable $2800
;   Y = 96-127 nametable $2C00
ppu_address_tile:
	lda $2002 ; reset latch
	tya
	lsr
	lsr
	lsr
	ora #$20 ; high bits of Y + $20
	sta $2006
	tya
	asl
	asl
	asl
	asl
	asl
	sta temp
	txa
	ora temp
	sta $2006 ; low bits of Y + X
	rts
	
;
; irq routine
;

.segment "CODE"
irq:
	rti
	
;
; gamepad
;
PAD_R      = $01
PAD_L      = $02
PAD_D      = $04
PAD_U      = $08
PAD_START  = $10
PAD_SELECT = $20
PAD_B      = $40
PAD_A      = $80

.segment "ZEROPAGE"
gamepad: .res 1

.segment "CODE"
gamepad_poll:
	; strobe gamepad
	lda #$01
	sta JOY1
	sta gamepad
	
	lda #$00
	sta JOY1
@loop:
	lda JOY1
	lsr a ; bit 0 -> carry flag
	rol gamepad ; carry -> bit 0; bit 7 -> carry
	bcc @loop
	
	rts
	
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
level_select:      .incbin "level_select.nam"
level_data:        .incbin "levels.bin"

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
	lda count_rollover
	cmp #1
	beq :+ ; branch if count_rollover == 1
	jsr gamepad_poll
	lda gamepad
	and #PAD_START
	bne :+ ; branch if start is pressed
	jmp :-
:
	lda #0
	sta count_rollover
	; draw level select screen
	jsr ppu_off
	;ldx #<level_select
	;ldy #>level_select
	;jsr draw_background
	lda #0
	jsr draw_level
	jsr ppu_update
	
@loop:
	jsr gamepad_poll
	lda gamepad
	
	jsr ppu_update
	jmp @loop
		

.segment "ZEROPAGE"
fill_addr: .res 2

.segment "CODE"
; set X/Y to the starting address of the nametable, X low byte, Y high byte
draw_background:
	stx fill_addr
	sty fill_addr+1

	lda PPUSTATUS ; reset latch
	lda #$20
	sta PPUADDR
	lda #$00
	sta PPUADDR
	
	ldx #0
	:
		ldy #0
		:
			lda (fill_addr), Y
			sta PPUDATA
			iny
			bne :-
		inc fill_addr+1
		inx
		cpx #$04
		bne :--

	rts
	
; set A to the level to draw
draw_level:
	; calculate address of level
	clc
	adc #>level_data
	sta fill_addr+1
	
	lda #<level_data
	sta fill_addr
	
	lda PPUSTATUS ; reset latch
	lda #$20
	sta PPUADDR
	lda #$00
	sta PPUADDR ; PPUADDR = $2000

	ldy #0
	@outer:
		ldx #0
		@inner:
			tya
			pha ; save y
			lsr
			asl
			asl
			asl
			asl
			sta temp
			txa
			lsr
			ora temp
			tay
			lda (fill_addr), Y
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
			cmp #$20 ; block tile
			bne :+
				lda #$34
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