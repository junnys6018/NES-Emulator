;
; util.asm
;

.include "nes.inc"
.include "global.inc"

; converts a binary number in the range 0..99 into decimal digits
; number to convert is stored in A register
; stores tens place in X register, ones place in Y register
bin_to_dec:
	ldx #0
	:
		cmp #10
		bcc @tens_done
		inx
		sbc #10
		jmp :-
@tens_done:
	tay	
	rts

; wait for nmi_count to reach $FF or until user presses start until function returns
wait_for_enter:
	@begin_wait:
		jsr gamepad_poll
		lda gamepad_trigger
		and #PAD_START
		bne @end_wait
		
		lda nmi_count
		cmp #$FF
		beq @end_wait
		
		jmp @begin_wait
	@end_wait:
	rts