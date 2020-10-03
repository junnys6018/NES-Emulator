#include "test_2C02.h"
#include "Backend/nes.h"
#include "test_util.h"

#include <stdio.h>

void RunAll2C02Tests()
{
#if 0
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/1.frame_basics.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/2.vbl_timing.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/3.even_odd_frames.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/4.vbl_clear_timing.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/5.nmi_suppression.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/6.nmi_disable.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/vbl_nmi_timing/7.nmi_timing.nes", 0xF8);

	TestBlarggRom("tests/roms/blargg_tests/blargg_ppu_tests_2005.09.15b/palette_ram.nes", 0xF0);
	TestBlarggRom("tests/roms/blargg_tests/blargg_ppu_tests_2005.09.15b/sprite_ram.nes", 0xF0);
	TestBlarggRom("tests/roms/blargg_tests/blargg_ppu_tests_2005.09.15b/vram_access.nes", 0xF0);
	//TestBlarggRom("tests/roms/blargg_tests/blargg_ppu_tests_2005.09.15b/vbl_clear_time.nes", 0xF0); // old version

	TestBlarggRom("tests/roms/blargg_tests/sprite_overflow_tests/1.Basics.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_overflow_tests/2.Details.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_overflow_tests/3.Timing.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_overflow_tests/4.Obscure.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_overflow_tests/5.Emulator.nes", 0xF8);
#endif
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/01.basics.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/02.alignment.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/03.corners.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/04.flip.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/05.left_clip.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/06.right_edge.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/07.screen_bottom.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/08.double_height.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/09.timing_basics.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/10.timing_order.nes", 0xF8);
	TestBlarggRom("tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/11.edge_timing.nes", 0xF8);
}
