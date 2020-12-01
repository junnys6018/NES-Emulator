;
; gamepad.asm
;
.include "nes.inc"
.include "global.inc"

.segment "ZEROPAGE"
gamepad_new:     .res 1 ; gamepad state in current frame
gamepad_old:     .res 1 ; gamepad state in last frame
gamepad_trigger: .res 1 ; triggered on rising edge

; gamepad_press: gives a "keyboard" effect when holding a key
gamepad_press:   .res 1
curr_press:      .res 1
timeout:         .res 1 ; number of frames before another key press
delay_long:      .res 1
delay_short:     .res 1

.segment "CODE"
; gamepad_poll: called once every nmi
gamepad_poll:
	; update gamepad_old
	lda gamepad_new
	sta gamepad_old
	
	; strobe gamepad
	lda #$01
	sta JOY1
	sta gamepad_new
	
	lda #$00
	sta JOY1
@loop:
	lda JOY1
	lsr a ; bit 0 -> carry flag
	rol gamepad_new ; carry -> bit 0; bit 7 -> carry
	bcc @loop
	
	; calculate gamepad_trigger
	lda gamepad_old
	eor #$FF ; invert bits
	and gamepad_new
	sta gamepad_trigger ; gamepad_trigger = ~gamepad_old & gamepad_new
	
	; calculate gamepad_press
	lda curr_press
	bne @on_press ; branch if we are "tracking" a button press
		; if we are net tracking a button, check if a button has been pressed for us to start tracking
		lda gamepad_new
		bne :+ ; no button press, rts
			rts
		:
		lda #$80
		sta curr_press
		lda delay_long
		sta timeout
		lda gamepad_new
		; shift gamepad_new left until carry bit is set, while shifting curr_press right
		; This has the effect of finding the first button press in order from left to right 
		; and setting curr_press to the first button found
		:
			asl
			bcs @done
			lsr curr_press
			jmp :-
	@done:
		lda curr_press
		sta gamepad_press
		rts
@on_press:
	lda gamepad_new
	and curr_press
	bne :+ ; check if button we are tracking has been released
		; if so, clear curr_press and gamepad_press
		lda #0
		sta curr_press
		sta gamepad_press
		rts
	:
	dec timeout
	bne :+ ; wait for timeout before setting gamepad_press again
		lda delay_short
		sta timeout
		jmp @done
	:
		lda #0
		sta gamepad_press
		rts
	