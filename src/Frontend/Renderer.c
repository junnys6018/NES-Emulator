#include "Renderer.h"

#include <SDL.h> 
#include <glad/glad.h>
#include <SDL_opengl.h>

#include <stdio.h>
#include <assert.h>

#include "Gui.h"
#include "StartupOptions.h"
#include "ColorDefs.h"

#include "OpenGL/Debug.h"
#include "OpenGL/Scanline.h"
#include "OpenGL/BatchRenderer.h"
#include "OpenGL/TextRenderer.h"

#include "Views/AboutView.h"
#include "Views/ApuWaveView.h"
#include "Views/MemoryView.h"
#include "Views/NesStateView.h"
#include "Views/SettingsView.h"
#include "Views/NesView.h"

#include "Models/ChannelEnableModel.h"
#include "Models/NameTableModel.h"
#include "Models/PaletteDataModel.h"
#include "Models/NesScreenModel.h"
#include "Models/SettingsModel.h"

///////////////////////////
//
// Section: Structs and enums
//
///////////////////////////

typedef enum
{
	TARGET_NES_STATE,
	TARGET_APU_OSC,
	TARGET_MEMORY,
	TARGET_ABOUT,
	TARGET_SETTINGS
} DrawTarget;

typedef struct
{
	SDL_Window* win;
	SDL_GLContext gl_context;

	// Window size metrics
	WindowMetrics wm;
	GuiMetrics gm;

	bool draw_debug_view;

	// What screen we are currently drawing
	DrawTarget target;

	// Pointer to the nes the renderer is drawing
	Nes* nes;

	// Models
	ChannelEnableModel m_channel_enable;
	NameTableModel m_nametable;
	PaletteDataModel m_palette;
	NesScreenModel m_nes_screen;
	SettingsModel m_settings;

} ControllerContext;

static ControllerContext cc;

///////////////////////////
//
// Section: Initialization
//
///////////////////////////

void CalculateWindowMetrics(int w, int h)
{
	glViewport(0, 0, w, h);
	cc.draw_debug_view = true;

	cc.wm.width = w;
	cc.wm.height = h;

	cc.wm.padding = lroundf(0.0045f * (w + h));

	if ((float)w / h >= 256.0f / 240.0f)
	{
		cc.wm.nes_x = cc.wm.padding;
		cc.wm.nes_y = cc.wm.padding;

		cc.wm.nes_h = h - 2 * cc.wm.padding;
		cc.wm.nes_w = lroundf(cc.wm.nes_h * 256.0f / 240.0f); // maintain aspect ratio
	}
	else
	{
		cc.wm.nes_w = w - 2 * cc.wm.padding;
		cc.wm.nes_h = lroundf(cc.wm.nes_w * 240.0f / 256.0f);

		cc.wm.nes_x = cc.wm.padding;
		cc.wm.nes_y = (h - cc.wm.nes_h) / 2;
		cc.draw_debug_view = false;
	}

	cc.wm.db_x = 2 * cc.wm.padding + cc.wm.nes_w;
	cc.wm.db_y = cc.wm.padding;


	cc.wm.db_w = w - 3 * cc.wm.padding - cc.wm.nes_w;
	cc.wm.db_h = h - 2 * cc.wm.padding;

	const int min_db_w = 200, min_db_h = 400;
	if (cc.wm.db_w < min_db_w || cc.wm.db_h < min_db_h)
	{
		cc.draw_debug_view = false;

		// Center the nes screen to take up the space cleared from not drawing debug screen
		cc.wm.nes_x = (w - cc.wm.nes_w) / 2;
		cc.wm.nes_y = (h - cc.wm.nes_h) / 2;
	}

	cc.wm.button_h = lroundf(0.03f * h);
	cc.wm.pattern_table_len = lroundf(0.096f * (w + h));

	cc.wm.menu_button_w = (float)cc.wm.db_w / 5.0f;
	cc.wm.menu_button_h = lroundf(0.0406f * h);

	cc.wm.palette_visual_len = lroundf(0.004f * (w + h));

	cc.wm.apu_osc_height = lroundf(0.1355f * h);
}

void InitOpengl(SDL_Window* window) 
{
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	cc.gl_context = SDL_GL_CreateContext(window);
	int success = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	if (!success)
	{
		printf("[ERROR] Failed to initialize opengl");
	}

	EnableGLDebugging();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.125, 0.125, 0.125, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	InitScanlineEffect();
	InitBatchRenderer();
	StartupOptions* opt = GetStartupOptions();
	InitTextRenderer(opt->font_style, opt->font_size);
}

void ShutdownOpengl()
{
	ShutdownBatchRenderer();
	ShutdownTextRenderer();
	ShutdownScanlineEffect();
	SDL_GL_DeleteContext(cc.gl_context);
}

void InitController()
{
	StartupOptions* opt = GetStartupOptions();
	const int starting_w = opt->startup_width, starting_h = opt->startup_height;

	cc.m_settings.mode = MODE_NOT_RUNNING;

	// Create a window
	Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
	cc.m_settings.fullscreen = opt->fullscreen_on_startup;
	if (opt->fullscreen_on_startup)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	
	cc.win = SDL_CreateWindow("NES Emulator - By Jun Lim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, starting_w, starting_h, flags);
	if (!cc.win)
	{
		printf("Could not create window");
	}

	InitOpengl(cc.win);
	CalculateWindowMetrics(starting_w, starting_h);

	// Create nametable textures
	GenerateTexture(&cc.m_nametable.left_nametable, 128, 128, NULL, GL_NEAREST, GL_RGB);
	ClearTexture(&cc.m_nametable.left_nametable);

	GenerateTexture(&cc.m_nametable.right_nametable, 128, 128, NULL, GL_NEAREST, GL_RGB);
	ClearTexture(&cc.m_nametable.right_nametable);

	// And main screen 
	GenerateTexture(&cc.m_nes_screen.scr, 256, 240, NULL, GL_NEAREST, GL_RGB);
	ClearTexture(&cc.m_nes_screen.scr);

	// Enable sound channels
	cc.m_channel_enable.SQ1 = true;
	cc.m_channel_enable.SQ2 = true;
	cc.m_channel_enable.TRI = true;
	cc.m_channel_enable.NOISE = true;
	cc.m_channel_enable.DMC = true;

	// GUI
	cc.gm.scroll_bar_width = 18;
	cc.gm.checkbox_size = 18;
	cc.gm.font_size = opt->font_size;
	cc.gm.padding = cc.wm.padding;
	GuiInit(&cc.gm);

	cc.target = TARGET_NES_STATE;
}

void ControllerShutdown()
{
	ShutdownOpengl();

	DeleteTexture(&cc.m_nametable.left_nametable);
	DeleteTexture(&cc.m_nametable.right_nametable);
	DeleteTexture(&cc.m_nes_screen.scr);

	SDL_DestroyWindow(cc.win);

	GuiShutdown();
}

///////////////////////////
//
// Section: Interfacing with the renderer
//
///////////////////////////

WindowMetrics* GetWindowMetrics()
{
	return &cc.wm;
}

Nes* GetBoundNes()
{
	return cc.nes;
}

void SetFullScreen(bool b)
{
	SDL_SetWindowFullscreen(cc.win, b ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

SettingsModel* GetSettings()
{
	return &cc.m_settings;
}

// Whenever a pattern table has been bank switched, the pointer the renderer has to the pattern table
// will be invalid, backend code will call this function to update that pointer to the new pattern table
void ControllerSetPatternTable(uint8_t* table_data, int side)
{
	if (side == 0)
	{
		cc.m_nametable.left_nt_data = table_data;
	}
	else
	{
		cc.m_nametable.right_nt_data = table_data;
	}
}

void ControllerBindNES(Nes* nes)
{
	cc.nes = nes;
	cc.m_palette.pal = nes->ppu_bus.palette;
}

// The PPU will call this function whenever a new frame is ready
void SendPixelDataToScreen(uint8_t* pixels)
{
	glTextureSubImage2D(cc.m_nes_screen.scr.handle, 0, 0, 0, 256, 240, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void GetWindowSize(int* w, int* h)
{
	SDL_GetWindowSize(cc.win, w, h);
}

///////////////////////////
//
// Section: Drawing the views
//
///////////////////////////

void ControllerDrawViews()
{
	assert(cc.nes); // Nes must be bound for rendering

	int w, h;
	SDL_GetWindowSize(cc.win, &w, &h);
	if (w != cc.wm.width || h != cc.wm.height)
	{
		CalculateWindowMetrics(w, h);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	BeginBatch();

	DrawNes(cc.m_nes_screen);
	
	// Draw 8x8 grid over nes screen for debugging
	//if (rc.draw_grid)
	//{
	//	for (int i = 0; i <= 32; i++)
	//	{
	//		if (i == 16)
	//			SDL_SetRenderDrawColor(rc.rend, 128, 128, 128, 255);
	//		else if (i % 2 == 0)
	//			SDL_SetRenderDrawColor(rc.rend, 64, 64, 64, 255);
	//		else
	//			SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 255);
	//		int x = rc.wm.nes_x + rc.wm.nes_w * i / 32;
	//		SDL_RenderDrawLine(rc.rend, x, rc.wm.nes_y, x, rc.wm.nes_y + rc.wm.nes_h);
	//	}
	//	for (int i = 0; i <= 30; i++)
	//	{
	//		if (i == 15)
	//			SDL_SetRenderDrawColor(rc.rend, 128, 128, 128, 255);
	//		else if (i % 2 == 0)
	//			SDL_SetRenderDrawColor(rc.rend, 64, 64, 64, 255);
	//		else
	//			SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 255);
	//		int y = rc.wm.nes_y + rc.wm.nes_w * i / 32;
	//		SDL_RenderDrawLine(rc.rend, rc.wm.nes_x, y, rc.wm.nes_x + rc.wm.nes_w, y);
	//	}
	//}

	if (cc.draw_debug_view)
	{
		SDL_Rect r_DebugView = {.x = cc.wm.db_x, .y = cc.wm.db_y, .w = cc.wm.db_w, .h = cc.wm.db_h};
		SubmitColoredQuad(&r_DebugView, 16, 16, 16);

		char* button_names[] = {"NES State", "APU Wave", "Memory", "About", "Settings"};
		DrawTarget targets[] = {TARGET_NES_STATE, TARGET_APU_OSC, TARGET_MEMORY, TARGET_ABOUT, TARGET_SETTINGS};
		int button_positions[6];

		button_positions[0] = cc.wm.db_x;
		button_positions[5] = cc.wm.db_x + cc.wm.db_w;
		for (int i = 1; i <= 4; i++)
		{
			button_positions[i] = lroundf(cc.wm.db_x + i * cc.wm.menu_button_w);
		}

		for (int i = 0; i < 5; i++)
		{
			SDL_Rect span;
			span.y = cc.wm.padding;
			span.h = cc.wm.menu_button_h;
			span.x = button_positions[i];
			span.w = button_positions[i + 1] - span.x;

			if (GuiAddButton(button_names[i], &span))
			{
				cc.target = targets[i];
			}
		}

		switch (cc.target)
		{
		case TARGET_NES_STATE:
			DrawNESState(&cc.m_nametable, cc.m_palette);
			break;
		case TARGET_APU_OSC:
			DrawAPUOsc(&cc.m_channel_enable);
			break;
		case TARGET_MEMORY:
			DrawMemoryView();
			break;
		case TARGET_ABOUT:
			DrawAbout();
			break;
		case TARGET_SETTINGS:
			DrawSettings(&cc.m_channel_enable, &cc.m_nes_screen, &cc.m_settings);
			break;
		}

		// Draw FPS
		SetTextOrigin(cc.wm.db_x + cc.wm.padding, cc.wm.db_y + cc.wm.db_h - GetStartupOptions()->font_size - cc.wm.padding);
		char buf[64];
		sprintf(buf, "%.3f ms/frame", cc.m_settings.ms_per_frame);
		RenderText(buf, white);
	}

	GuiEndFrame();
	EndBatch();

	// Swap framebuffers
	SDL_GL_SwapWindow(cc.win);
}

#if 0
// For debugging
void DrawNametable(State2C02* ppu)
{
	color* pixels;
	color col[4] = { {0,0,0},{255,0,0,},{0,0,0,},{255,0,0} };
	int pitch;
	SDL_LockTexture(cc.nes_screen, NULL, &pixels, &pitch);

	memset(pixels, 255, 256 * 240 * 3);

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 30; y++)
		{
			uint16_t nt_addr = 0x2000 | (y * 32 + x);
			uint8_t nt_byte = ppu_bus_read(ppu->bus, nt_addr);
			// Draw 8x8 tile
			for (int fine_y = 0; fine_y < 8; fine_y++)
			{
				uint16_t pt_addr = (uint16_t)(nt_byte << 4) | fine_y;
				uint8_t pt_byte_low = ppu_bus_read(ppu->bus, pt_addr);
				uint8_t pt_byte_high = ppu_bus_read(ppu->bus, pt_addr | 1 << 3);
				for (int fine_x = 0; fine_x < 8; fine_x++)
				{
					int yoff = y * 8 + fine_y;
					int xoff = x * 8 + 7 - fine_x;
					int index = yoff * 256 + xoff;
					int shade = (pt_byte_low & (1 << fine_x)) >> fine_x | pt_byte_high & (1 << fine_x) >> (fine_x - 1);
					pixels[index] = col[shade];
					if (fine_x == 0 || fine_y == 0)
					{
						pixels[index].r = 100;
						pixels[index].g = 100;
						pixels[index].b = 100;
					}
				}
			}
		}
	}

	SDL_UnlockTexture(cc.nes_screen);
}
#endif