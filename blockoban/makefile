all: blockoban.nes

levels.bin: levels.txt compile_levels.py
	python compile_levels.py

blockoban.o: blockoban.asm background.chr sprites.chr opening_screen.nam level_select.nam palette.pal levels.bin
	ca65 blockoban.asm

blockoban.nes: blockoban.o blockoban.cfg
	ld65 -C blockoban.cfg -o blockoban.nes blockoban.o
	
clean:
	rm -f blockoban.nes blockoban.o levels.bin