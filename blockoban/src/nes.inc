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

PAD_R          = $01
PAD_L          = $02
PAD_D          = $04
PAD_U          = $08
PAD_START      = $10
PAD_SELECT     = $20
PAD_B          = $40
PAD_A          = $80

; Define metatile indices

TILE_WALL      = $30
TILE_CRATE     = $34
TILE_FLAG      = $38
TILE_BUTTON    = $3C
TILE_AIR       = $FC

NUM_LEVELS = 4

; character mapping
.charmap '0', 0
.charmap '1', 1
.charmap '2', 2
.charmap '3', 3
.charmap '4', 4
.charmap '5', 5
.charmap '6', 6
.charmap '7', 7
.charmap '8', 8
.charmap '9', 9
.charmap 'A', 10
.charmap 'B', 11
.charmap 'C', 12
.charmap 'D', 13
.charmap 'E', 14
.charmap 'F', 15
.charmap 'G', 16
.charmap 'H', 17
.charmap 'I', 18
.charmap 'J', 19
.charmap 'K', 20
.charmap 'L', 21
.charmap 'M', 22
.charmap 'N', 23
.charmap 'O', 24
.charmap 'P', 25
.charmap 'Q', 26
.charmap 'R', 27
.charmap 'S', 28
.charmap 'T', 29
.charmap 'U', 30
.charmap 'V', 31
.charmap 'W', 32
.charmap 'X', 33
.charmap 'Y', 34
.charmap 'Z', 35
.charmap ' ', 255

; some macros

.macro addr_jsr addr, routine
	ldx #<addr
	ldy #>addr
	jsr routine
.endmacro