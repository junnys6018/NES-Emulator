;
; irq.asm
;

.include "nes.inc"
.include "global.inc"

.segment "CODE"
irq:
	rti