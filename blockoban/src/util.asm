;
; utility
;

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
