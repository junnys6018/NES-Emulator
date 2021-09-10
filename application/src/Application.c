#include "Application.h"

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "Audio.h"
#include "ColorDefs.h"
#include "GameController.h"
#include "Gui.h"
#include "StartupOptions.h"
#include "Util/timer.h"

#include "OpenGL/BatchRenderer.h"
#include "OpenGL/Debug.h"
#include "OpenGL/LineRenderer.h"
#include "OpenGL/Scanline.h"
#include "OpenGL/TextRenderer.h"

#include "Views/AboutView.h"
#include "Views/ApuWaveView.h"
#include "Views/MemoryView.h"
#include "Views/NesStateView.h"
#include "Views/NesView.h"
#include "Views/SettingsView.h"

#include "Models/ChannelEnableModel.h"
#include "Models/NameTableModel.h"
#include "Models/NesScreenModel.h"
#include "Models/PaletteDataModel.h"
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

	Nes* nes;
	GameController game_controller;

	// Models
	ChannelEnableModel m_channel_enable;
	NameTableModel m_nametable;
	PaletteDataModel m_palette;
	NesScreenModel m_nes_screen;
	SettingsModel m_settings;

} ApplicationContext;

static ApplicationContext ac;

///////////////////////////
//
// Section: Initialization
//
///////////////////////////

void CalculateWindowMetrics(int w, int h)
{
	glViewport(0, 0, w, h);
	ac.draw_debug_view = true;

	ac.wm.width = w;
	ac.wm.height = h;

	ac.wm.padding = lroundf(0.0045f * (w + h));

	if ((float)w / h >= 256.0f / 240.0f)
	{
		ac.wm.nes_x = ac.wm.padding;
		ac.wm.nes_y = ac.wm.padding;

		ac.wm.nes_h = h - 2 * ac.wm.padding;
		ac.wm.nes_w = lroundf(ac.wm.nes_h * 256.0f / 240.0f); // maintain aspect ratio
	}
	else
	{
		ac.wm.nes_w = w - 2 * ac.wm.padding;
		ac.wm.nes_h = lroundf(ac.wm.nes_w * 240.0f / 256.0f);

		ac.wm.nes_x = ac.wm.padding;
		ac.wm.nes_y = (h - ac.wm.nes_h) / 2;
		ac.draw_debug_view = false;
	}

	ac.wm.db_x = 2 * ac.wm.padding + ac.wm.nes_w;
	ac.wm.db_y = ac.wm.padding;

	ac.wm.db_w = w - 3 * ac.wm.padding - ac.wm.nes_w;
	ac.wm.db_h = h - 2 * ac.wm.padding;

	const int min_db_w = 200, min_db_h = 400;
	if (ac.wm.db_w < min_db_w || ac.wm.db_h < min_db_h)
	{
		ac.draw_debug_view = false;

		// Center the nes screen to take up the space cleared from not drawing debug screen
		ac.wm.nes_x = (w - ac.wm.nes_w) / 2;
		ac.wm.nes_y = (h - ac.wm.nes_h) / 2;
	}

	ac.wm.button_h = lroundf(0.03f * h);
	ac.wm.pattern_table_len = lroundf(0.096f * (w + h));

	ac.wm.menu_button_w = (float)ac.wm.db_w / 5.0f;
	ac.wm.menu_button_h = lroundf(0.0406f * h);

	ac.wm.palette_visual_len = lroundf(0.004f * (w + h));

	ac.wm.apu_osc_height = lroundf(0.1355f * h);
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

	ac.gl_context = SDL_GL_CreateContext(window);
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
	InitLineRenderer();
	InitBatchRenderer();
	StartupOptions* opt = GetStartupOptions();
	InitTextRenderer(opt->font_style, opt->font_size);
}

void ShutdownOpengl()
{
	ShutdownBatchRenderer();
	ShutdownTextRenderer();
	ShutdownLineRenderer();
	ShutdownScanlineEffect();
	SDL_GL_DeleteContext(ac.gl_context);
}

void InitApplication(char* rom)
{
	char error_string[256];
	ac.nes = malloc(sizeof(Nes));

	int result = initialize_nes(ac.nes, rom, SetPatternTable, error_string);
	if (result != 0)
	{
		printf("[ERROR]: %s\n", error_string);
		ac.m_settings.mode = MODE_NOT_RUNNING;
	}
	else
	{
		ac.m_settings.mode = MODE_PLAY;
	}

	if (!rom)
		ac.m_settings.mode = MODE_NOT_RUNNING;

	ac.m_palette.pal = ac.nes->ppu_bus.palette;

	StartupOptions* opt = GetStartupOptions();
	const int starting_w = opt->startup_width, starting_h = opt->startup_height;

	// Attempt to open a game controller
	TryOpenGameController(&ac.game_controller);

	// Create a window
	Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
	ac.m_settings.fullscreen = opt->fullscreen_on_startup;
	if (opt->fullscreen_on_startup)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	ac.win = SDL_CreateWindow("NES Emulator - By Jun Lim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, starting_w, starting_h, flags);
	if (!ac.win)
	{
		printf("Could not create window");
	}

	InitOpengl(ac.win);
	CalculateWindowMetrics(starting_w, starting_h);

	// Create nametable textures
	GenerateTexture(&ac.m_nametable.left_nametable, 128, 128, NULL, GL_NEAREST, GL_RGB);
	ClearTexture(&ac.m_nametable.left_nametable);

	GenerateTexture(&ac.m_nametable.right_nametable, 128, 128, NULL, GL_NEAREST, GL_RGB);
	ClearTexture(&ac.m_nametable.right_nametable);

	// And main screen
	GenerateTexture(&ac.m_nes_screen.scr, 256, 240, NULL, GL_NEAREST, GL_RGB);
	ClearTexture(&ac.m_nes_screen.scr);

	// Enable sound channels
	ac.m_channel_enable.SQ1 = true;
	ac.m_channel_enable.SQ2 = true;
	ac.m_channel_enable.TRI = true;
	ac.m_channel_enable.NOISE = true;
	ac.m_channel_enable.DMC = true;

	// GUI
	ac.gm.scroll_bar_width = 18;
	ac.gm.checkbox_size = 18;
	ac.gm.font_size = opt->font_size;
	ac.gm.padding = ac.wm.padding;
	GuiInit(&ac.gm);

	ac.target = TARGET_NES_STATE;
}

void ShutdownApplication()
{
	ShutdownOpengl();

	DeleteTexture(&ac.m_nametable.left_nametable);
	DeleteTexture(&ac.m_nametable.right_nametable);
	DeleteTexture(&ac.m_nes_screen.scr);

	SDL_DestroyWindow(ac.win);

	GuiShutdown();

	free(ac.nes);

	CloseGameController(&ac.game_controller);
}

///////////////////////////
//
// Section: Interfacing with the renderer
//
///////////////////////////

WindowMetrics* GetWindowMetrics()
{
	return &ac.wm;
}

Nes* GetApplicationNes()
{
	return ac.nes;
}

void SetFullScreen(bool b)
{
	SDL_SetWindowFullscreen(ac.win, b ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

// Whenever a pattern table has been bank switched, the pointer the renderer has to the pattern table
// will be invalid, backend code will call this function to update that pointer to the new pattern table
void SetPatternTable(uint8_t* table_data, int side)
{
	if (side == 0)
	{
		ac.m_nametable.left_nt_data = table_data;
	}
	else
	{
		ac.m_nametable.right_nt_data = table_data;
	}
}

void GetWindowSize(int* w, int* h)
{
	SDL_GetWindowSize(ac.win, w, h);
}

///////////////////////////
//
// Section: Drawing the views
//
///////////////////////////

void DrawViews()
{
	assert(ac.nes); // Nes must be bound for rendering

	int w, h;
	SDL_GetWindowSize(ac.win, &w, &h);
	if (w != ac.wm.width || h != ac.wm.height)
	{
		CalculateWindowMetrics(w, h);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	BeginBatch();
	BeginLines();

	uint32_t* pixels = get_framebuffer(&ac.nes->ppu);
	glTextureSubImage2D(ac.m_nes_screen.scr.handle, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	DrawNes(ac.m_nes_screen, &ac.m_settings);

	if (ac.draw_debug_view)
	{
		SDL_Rect r_DebugView = {.x = ac.wm.db_x, .y = ac.wm.db_y, .w = ac.wm.db_w, .h = ac.wm.db_h};
		SubmitColoredQuad(&r_DebugView, 16, 16, 16);

		char* button_names[] = {"NES State", "APU Wave", "Memory", "About", "Settings"};
		DrawTarget targets[] = {TARGET_NES_STATE, TARGET_APU_OSC, TARGET_MEMORY, TARGET_ABOUT, TARGET_SETTINGS};
		int button_positions[6];

		button_positions[0] = ac.wm.db_x;
		button_positions[5] = ac.wm.db_x + ac.wm.db_w;
		for (int i = 1; i <= 4; i++)
		{
			button_positions[i] = lroundf(ac.wm.db_x + i * ac.wm.menu_button_w);
		}

		for (int i = 0; i < 5; i++)
		{
			SDL_Rect span;
			span.y = ac.wm.padding;
			span.h = ac.wm.menu_button_h;
			span.x = button_positions[i];
			span.w = button_positions[i + 1] - span.x;

			if (GuiAddButton(button_names[i], &span))
			{
				ac.target = targets[i];
			}
		}

		switch (ac.target)
		{
		case TARGET_NES_STATE:
			DrawNESState(&ac.m_nametable, ac.m_palette);
			break;
		case TARGET_APU_OSC:
			DrawAPUOsc(&ac.m_channel_enable);
			break;
		case TARGET_MEMORY:
			DrawMemoryView();
			break;
		case TARGET_ABOUT:
			DrawAbout(GetControllerType(&ac.game_controller));
			break;
		case TARGET_SETTINGS:
			DrawSettings(&ac.m_channel_enable, &ac.m_nes_screen, &ac.m_settings);
			break;
		}

		// Draw FPS
		SetTextOrigin(ac.wm.db_x + ac.wm.padding, ac.wm.db_y + ac.wm.db_h - GetStartupOptions()->font_size - ac.wm.padding);
		char buf[64];
		sprintf(buf, "%.3f ms/frame", ac.m_settings.ms_per_frame);
		RenderText(buf, white);
	}

	GuiEndFrame();
	EndBatch();
	EndLines();

	// Swap framebuffers
	SDL_GL_SwapWindow(ac.win);
}

///////////////////////////
//
// Section: Application Stuff
//
///////////////////////////

void OnControllerDeviceAdded(SDL_ControllerDeviceEvent* e)
{
	if (!IsGameControllerOpen(&ac.game_controller))
	{
		OpenGameController(&ac.game_controller, e->which);
	}
}

void OnControllerDeviceRemoved(SDL_ControllerDeviceEvent* e)
{
	SDL_GameController* gc = SDL_GameControllerFromInstanceID(e->which);
	if (gc == ac.game_controller.sdl_game_controller) // Current Controller was disconnected
	{
		CloseGameController(&ac.game_controller);
	}
}

void SetNesKeys()
{
	static const int8_t* state = NULL;
	if (!state)
	{
		state = SDL_GetKeyboardState(NULL);
	}

	StartupOptions* opt = GetStartupOptions();

	Keys keyboard_keys, controller_keys, keys;

	keyboard_keys.keys.A = state[opt->key_A];
	keyboard_keys.keys.B = state[opt->key_B];
	keyboard_keys.keys.Start = state[opt->key_start]; // Enter key
	keyboard_keys.keys.Select = state[opt->key_select];
	keyboard_keys.keys.Up = state[opt->key_up];
	keyboard_keys.keys.Down = state[opt->key_down];
	keyboard_keys.keys.Left = state[opt->key_left];
	keyboard_keys.keys.Right = state[opt->key_right];

	controller_keys = PollGameController(&ac.game_controller);

	keys.reg = keyboard_keys.reg | controller_keys.reg;
	poll_keys(&ac.nes->pad, keys);
}

void ApplicationGameLoop()
{
	int window = 10;
	float total_time = 0.0f;
	int curr_frame = 0;

	SDL_Event event;
	timepoint beg, end;
	bool running = true;
	while (running)
	{
		get_time(&beg);
		SetNesKeys();

		if (ac.m_settings.mode == MODE_PLAY)
		{
			clock_nes_frame(ac.nes);
			if (ac.nes->apu.audio_pos != 0)
			{
				WriteSamples(ac.nes->apu.audio_buffer, ac.nes->apu.audio_pos);
				ac.nes->apu.audio_pos = 0;
			}
		}

		DrawViews();

		while (SDL_PollEvent(&event) != 0)
		{
			GuiDispatchEvent(&event);
			if (event.type == SDL_CONTROLLERDEVICEADDED)
			{
				OnControllerDeviceAdded(&event.cdevice);
			}
			else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
			{
				OnControllerDeviceRemoved(&event.cdevice);
			}
			else if (event.type == SDL_KEYDOWN && ac.m_settings.mode == MODE_STEP_THROUGH)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					clock_nes_instruction(ac.nes);
					break;
				case SDLK_f:
					clock_nes_frame(ac.nes);
					break;
				case SDLK_p:
					clock_nes_cycle(ac.nes);
					break;
				}
			}
			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		get_time(&end);
		total_time += get_elapsed_time_milli(&beg, &end);
		curr_frame++;
		if (curr_frame == window)
		{
			ac.m_settings.ms_per_frame = total_time / window;

			total_time = 0.0f;
			curr_frame = 0;
		}

		float elapsed = get_elapsed_time_micro(&beg, &end);
		if (elapsed < 16666) // 60 FPS
		{
			sleep_micro((uint64_t)(16666 - elapsed));
		}
	}
}
