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