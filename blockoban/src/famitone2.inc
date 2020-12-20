.ifndef FAMITONE2_INC
.define FAMITONE2_INC

FT_BASE_ADR	    	= $0600	;page in the RAM used for FT2 variables, should be $xx00
FT_TEMP		     	= $FD	;3 bytes in zeropage used by the library as a scratchpad
FT_DPCM_OFF		    = $c000	;$c000..$ffc0, 64-byte steps
FT_SFX_STREAMS	    = 4		;number of sound effects played at once, 1..4

FT_DPCM_ENABLE      = 0		;undefine to exclude all DMC code
FT_SFX_ENABLE       = 1		;undefine to exclude all sound effects code
FT_THREAD           = 1		;undefine if you are calling sound effects from the same thread as the sound update call

FT_PAL_SUPPORT      = 0		;undefine to exclude PAL support
FT_NTSC_SUPPORT     = 1		;undefine to exclude NTSC support

FT_PITCH_FIX        = 0

;aliases for sound effect channels to use in user calls

FT_SFX_STRUCT_SIZE	= 15

FT_SFX_CH0			= FT_SFX_STRUCT_SIZE*0
FT_SFX_CH1			= FT_SFX_STRUCT_SIZE*1
FT_SFX_CH2			= FT_SFX_STRUCT_SIZE*2
FT_SFX_CH3			= FT_SFX_STRUCT_SIZE*3

PLAY_DING  = 0
PLAY_CRASH = 1
PLAY_CLICK = 2


.global FamiToneInit
.global FamiToneSfxInit
.global FamiToneUpdate
.global FamiToneMusicPlay
.global FamiToneMusicStop
.global FamiToneMusicPause
.global FamiToneSfxPlay

.endif