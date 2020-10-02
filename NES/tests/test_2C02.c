#include "test_2C02.h"
#include "Backend/nes.h"
#include "string_util.h"
#include "test_util.h"

#include <stdio.h>

void TestBlarggRom(const char* name)
{
	Nes nes;
	NESInit(&nes, name);

	RendererBindNES(&nes);

	EmulateUntilHalt(&nes, 100000);

	// Check for success or failure
	uint16_t result_addr = 0xF8;
	uint8_t result = cpu_bus_read(&nes.cpu_bus, result_addr);
	if (result == 1)
	{
		printf("%s passed!\n", GetFileName(name));
	}
	else
	{
		printf("%s failed [%i]\n", GetFileName(name), result);
	}
	
	RendererDraw();

	NESDestroy(&nes);
}


void RunAll2C02Tests()
{
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/1.frame_basics.nes");
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/2.vbl_timing.nes");
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/3.even_odd_frames.nes");
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/4.vbl_clear_timing.nes");
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/5.nmi_suppression.nes");
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/6.nmi_disable.nes");
	TestBlarggRom("tests/roms/blargg_vbl_nmi_timing/7.nmi_timing.nes");
}
