#include "SettingsView.h"
#include "Common.h"
#include "Util/FileDialog.h"
#include <stdio.h>

void SetAPUChannels(ChannelEnableModel* ch)
{
	State2A03* apu = &GetApplicationNes()->apu;

	apu_channel_set(apu, CHANNEL_SQ1, ch->SQ1);
	apu_channel_set(apu, CHANNEL_SQ2, ch->SQ2);
	apu_channel_set(apu, CHANNEL_TRI, ch->TRI);
	apu_channel_set(apu, CHANNEL_NOISE, ch->NOISE);
	apu_channel_set(apu, CHANNEL_DMC, ch->DMC);
}

void DrawSettings(ChannelEnableModel* ch, NesScreenModel* scr, SettingsModel* settings)
{
	Nes* nes = GetApplicationNes();
	GuiMetrics* gm = GetGuiMetrics();
	WindowMetrics* wm = GetWindowMetrics();
	int xoff = wm->db_x;
	int yoff = wm->db_y + wm->menu_button_h;
	int padding = wm->padding;

	SDL_Rect span = {xoff + padding, yoff + padding, 0.09195f * wm->width, wm->button_h};

	if (GuiAddButton("Load ROM...", &span))
	{
		char file[256];
		if (OpenFileDialog(file, 256) == 0)
		{
			destroy_nes(nes);
			char error_string[256];

			if (initialize_nes(nes, file, SetPatternTable, error_string) != 0)
			{
				printf("[ERROR]: %s\n", error_string);

				// Failed to load rom
				settings->mode = MODE_NOT_RUNNING;

				// ...so load a dummy one
				initialize_nes(nes, NULL, SetPatternTable, NULL);
			}
			// Successfully loaded rom
			else if (settings->mode == MODE_NOT_RUNNING)
			{
				settings->mode = MODE_PLAY;
			}
			SetAPUChannels(ch);
		}
		ClearTexture(&scr->scr);
	}
	span.y = yoff + 2 * padding + wm->button_h;
	span.w = 0.1494f * wm->width;
	if (GuiAddButton(settings->mode == MODE_PLAY ? "Step Through Emulation" : "Run Emulation", &span) && settings->mode != MODE_NOT_RUNNING)
	{
		// Toggle between Step Through and Playing
		settings->mode = 1 - settings->mode;
	}
	span.y += padding + wm->button_h;
	span.w = 0.09195f * wm->width;
	if (GuiAddButton("Reset", &span))
	{
		ClearTexture(&scr->scr);
		reset_nes(nes);
		SetAPUChannels(ch); // Configue APU channels to current settings
	}
	//span.y += padding + wm->button_h;
	//if (GuiAddButton("Save Game", &span))
	//{
	//}
	//span.y += padding + wm->button_h;
	//if (GuiAddButton("Restore Game", &span))
	//{
	//}
	span.y += padding + wm->button_h;

	GuiAddCheckbox("Draw Grid", span.x, span.y, &settings->draw_grid);
	span.y += padding + gm->checkbox_size;

	if (GuiAddCheckbox("Fullscreen", span.x, span.y, &settings->fullscreen))
	{
		SetFullScreen(settings->fullscreen);
	}
	span.y += padding + gm->checkbox_size;

	if (GuiAddCheckbox("Scanline Effect", span.x, span.y, &settings->scanline))
	{
		glBindTexture(GL_TEXTURE_2D, scr->scr.handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings->scanline ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings->scanline ? GL_LINEAR : GL_NEAREST);
	}
	span.y += padding + gm->checkbox_size;

	SetTextOrigin(xoff + padding, span.y);
	RenderText("ROM Infomation", cyan);
	Header header = nes->cart.header;
	char buf[64];

	sprintf(buf, "Mapper        - %u", nes->cart.mapper_id);
	RenderText(buf, white);

	int PRG_banks = ((uint16_t)header.PRGROM_MSB << 8) | (uint16_t)header.PRGROM_LSB;
	sprintf(buf, "PRG ROM banks - %i [%iKB]", PRG_banks, PRG_banks * 16);
	RenderText(buf, white);

	int CHR_banks = ((uint16_t)header.CHRROM_MSB << 8) | (uint16_t)header.CHRROM_LSB;
	sprintf(buf, "CHR ROM banks - %i [%iKB]", CHR_banks, CHR_banks * 8);
	RenderText(buf, white);

	sprintf(buf, "PPU mirroring - %s", header.mirror_type ? "vertical" : "horizontal");
	RenderText(buf, white);

	sprintf(buf, "Battery       - %s", header.battery ? "yes" : "no");
	RenderText(buf, white);

	sprintf(buf, "Trainer       - %s", header.trainer ? "yes" : "no");
	RenderText(buf, white);

	sprintf(buf, "4-Screen      - %s", header.four_screen ? "yes" : "no");
	RenderText(buf, white);
}