SRC_DIR = src/
INCLUDES = $(SRC_DIR)nes.inc $(SRC_DIR)global.inc
BINARIES = background.chr sprites.chr opening_screen.nam level_select.nam palette.pal levels.bin
OBJ_FILES = blockoban.o nmi.o gamepad.o reset.o

all: blockoban.nes

levels.bin: levels.txt compile_levels.py
	python compile_levels.py
	
%.o: $(SRC_DIR)%.asm $(INCLUDES)
	ca65 $< -o $@
	
blockoban.o: $(SRC_DIR)blockoban.asm $(BINARIES) $(INCLUDES)
	ca65 $(SRC_DIR)blockoban.asm -o blockoban.o

blockoban.nes: $(OBJ_FILES) $(SRC_DIR)blockoban.cfg
	ld65 -C $(SRC_DIR)blockoban.cfg -o blockoban.nes $(OBJ_FILES)
	
clean:
	rm -f blockoban.nes $(OBJ_FILES) levels.bin