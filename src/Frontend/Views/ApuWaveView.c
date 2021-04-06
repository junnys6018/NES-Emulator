#include "ApuWaveView.h"
#include "Common.h"
#include "../../Backend/2A03.h"

// A trigger function takes an audio buffer and returns an offset into that buffer
// when drawing a wave in DrawWaveform(), we start drawing from this offset.
// we do this to track the wave, otherwise the waveform will be jumping around in the visualisation
// The pulse and triangle channels each require a different triggering algortihm, so we use a function pointer.
typedef int (*TRIGGER_FUNC)(AudioWindow* win);

// Find a rising edge
int pulse_trigger(AudioWindow* win)
{
	float max_slope = -100000.0f;
	int max_x = 0;
	for (int x = 0; x < 1024; x++)
	{
		float slope = win->buffer[(win->write_pos + 511 + x) % 2048] - win->buffer[(win->write_pos + 513 + x) % 2048];
		if (slope > max_slope)
		{
			max_slope = slope;
			max_x = x;
		}
	}

	return max_x;
}

// Find when the waveform crosses the x-axis from below
int triangle_trigger(AudioWindow* win)
{
	int zero_x = 0;
	float zero_sample = 100000.0f;
	for (int x = 0; x < 1024; x++)
	{
		float slope = win->buffer[(win->write_pos + 500 + x) % 2048] - win->buffer[(win->write_pos + 524 + x) % 2048];
		float abs_sample = win->buffer[(win->write_pos + 512 + x) % 2048];
		abs_sample = abs_sample < 0.0f ? -abs_sample : abs_sample;
		if (slope > 0 && abs_sample < zero_sample)
		{
			zero_sample = abs_sample;
			zero_x = x;
		}
	}

	return zero_x;
}

int no_trigger(AudioWindow* win)
{
	return 0;
}

// Visualise audio buffer as a waveform
void DrawWaveform(SDL_Rect* rect, AudioWindow* win, float vertical_scale, TRIGGER_FUNC trigger)
{
	static SDL_Color waveform_colour = {247, 226, 64}; // yellow

	// Clear the part of the screen where we are drawing the waveform
	SubmitColoredQuad(rect, 0, 0, 0);

	// Draw x and y axis
	int mid_y = rect->y + rect->h / 2;
	int mid_x = rect->x + rect->w / 2;
	SubmitLine(rect->x, mid_y, rect->x + rect->w, mid_y, 128, 128, 128);
	SubmitLine(mid_x, rect->y, mid_x, rect->y + rect->h, 128, 128, 128);

	// Find the offset into the audio buffer to start drawing from
	int trigger_x = trigger(win);

	// The audio buffer is a ring buffer, win->write_pos is the starting index into the ring buffer. The ring buffer contains 2048 samples
	int start_index = (win->write_pos + trigger_x) % 2048;
	float curr_index = 0.0f;

	// We want to visualise 1024 audio samples over rect->w pixels, so for each pixel we
	// increment by index_inc amount into the audio buffer. We are effectivly using a
	// nearest neighbour filter to filter 1024 samples into rect->w samples
	float index_inc = 1024.0f / (float)rect->w;

	// For each pixel
	int last_y = win->buffer[lroundf(curr_index + start_index) % 2048] * vertical_scale + mid_y;
	curr_index += index_inc;
	for (int i = 1; i < rect->w; i++)
	{
		int curr_y = win->buffer[lroundf(curr_index + start_index) % 2048] * vertical_scale + mid_y;
		curr_index += index_inc;
		SubmitLine(rect->x + i - 1, last_y, rect->x + i, curr_y, waveform_colour.r, waveform_colour.g, waveform_colour.b);
		last_y = curr_y;
	}
}

void DrawAPUOsc(ChannelEnableModel* model)
{
	State2A03* apu = &GetBoundNes()->apu;
	GuiMetrics* gm = GetGuiMetrics();
	WindowMetrics* wm = GetWindowMetrics();
	int xoff = wm->db_x;
	int yoff = wm->db_y + wm->menu_button_h;
	int padding = wm->padding;
	
	if (GuiAddCheckbox("Square 1", xoff + padding, yoff + padding, &model->SQ1))
	{
		apu_channel_set(apu, CHANNEL_SQ1, model->SQ1);
	}
	const int height = wm->apu_osc_height;

	SDL_Rect rect = {.x = xoff + padding, .y = yoff + 2 * padding + gm->checkbox_size, .w = wm->db_w - 2 * padding, .h = height};
	DrawWaveform(&rect, &apu->SQ1_win, 350.0f, pulse_trigger);

	int curr_height = yoff + 3 * padding + gm->checkbox_size + height;
	if (GuiAddCheckbox("Square 2", xoff + padding, curr_height, &model->SQ2))
	{
		apu_channel_set(apu, CHANNEL_SQ2, model->SQ2);
	}
	curr_height += padding + gm->checkbox_size;
	rect.y = curr_height;
	DrawWaveform(&rect, &apu->SQ2_win, 350.0f, pulse_trigger);

	curr_height += padding + height;
	if (GuiAddCheckbox("Triangle", xoff + padding, curr_height, &model->TRI))
	{
		apu_channel_set(apu, CHANNEL_TRI, model->TRI);
	}
	curr_height += padding + gm->checkbox_size;
	rect.y = curr_height;
	DrawWaveform(&rect, &apu->TRI_win, 200.0f, triangle_trigger);

	curr_height += padding + height;
	if (GuiAddCheckbox("Noise", xoff + padding, curr_height, &model->NOISE))
	{
		apu_channel_set(apu, CHANNEL_NOISE, model->NOISE);
	}
	curr_height += padding + gm->checkbox_size;
	rect.y = curr_height;
	DrawWaveform(&rect, &apu->NOISE_win, 200.0f, no_trigger);

	curr_height += padding + height;
	if (GuiAddCheckbox("DMC", xoff + padding, curr_height, &model->DMC))
	{
		apu_channel_set(apu, CHANNEL_DMC, model->DMC);
	}

#if 0
	curr_height += padding + gm->checkbox_size;
	SetTextOrigin(xoff + padding, curr_height);
	char buf[64];

	sprintf(buf, "Period: %i Timer: %i", rc.nes->apu.DMC_timer.period, rc.nes->apu.DMC_timer.counter);
	RenderText(buf, white);

	sprintf(buf, "sample addr: $%.4X sample len: %i", 0xC000 | (rc.nes->apu.DMC_ADDR << 6), (rc.nes->apu.DMC_LENGTH << 4) + 1);
	RenderText(buf, white);

	sprintf(buf, "output: %i remaining: %i", rc.nes->apu.DMC_LOAD_COUNTER, rc.nes->apu.DMC_memory_reader.bytes_remaining);
	RenderText(buf, white);
#endif
}