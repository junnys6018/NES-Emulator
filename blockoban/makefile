SRC_DIR = src/
OBJ_DIR = bin/
RES_DIR = res/

INCLUDES  = $(SRC_DIR)nes.inc
INCLUDES += $(SRC_DIR)global.inc

BINARIES  = $(RES_DIR)background.chr
BINARIES += $(RES_DIR)sprites.chr
BINARIES += $(RES_DIR)opening_screen.nam
BINARIES += $(RES_DIR)end_screen.nam
BINARIES += $(RES_DIR)level_select.nam
BINARIES += $(RES_DIR)palette.pal
BINARIES += $(OBJ_DIR)levels.bin

OBJ_FILES  = $(OBJ_DIR)blockoban.o
OBJ_FILES += $(OBJ_DIR)nmi.o
OBJ_FILES += $(OBJ_DIR)irq.o
OBJ_FILES += $(OBJ_DIR)gamepad.o
OBJ_FILES += $(OBJ_DIR)reset.o
OBJ_FILES += $(OBJ_DIR)util.o
OBJ_FILES += $(OBJ_DIR)drawing.o 
OBJ_FILES += $(OBJ_DIR)vars.o
OBJ_FILES += $(OBJ_DIR)level.o
OBJ_FILES += $(OBJ_DIR)end_screen.o

all: blockoban.nes

blockoban.nes: $(OBJ_FILES) $(SRC_DIR)blockoban.cfg | $(OBJ_DIR)
	ld65 -C $(SRC_DIR)blockoban.cfg -o $(OBJ_DIR)blockoban.nes $(OBJ_FILES)

$(OBJ_DIR)levels.bin: levels.txt compile_levels.py | $(OBJ_DIR)
	python compile_levels.py
	
$(OBJ_DIR)%.o: $(SRC_DIR)%.asm $(INCLUDES) | $(OBJ_DIR)
	ca65 $< -o $@
	
$(OBJ_DIR)blockoban.o: $(SRC_DIR)blockoban.asm $(BINARIES) $(INCLUDES) | $(OBJ_DIR)
	ca65 $(SRC_DIR)blockoban.asm -o $(OBJ_DIR)blockoban.o
	
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	
clean:
	rm -rf $(OBJ_DIR)