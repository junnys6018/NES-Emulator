#include "2A03.h"
#include "nes.h" // For sample rate
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

enum
{
	SQ1 = 0,
	SQ2 = 1,
};

// Look up tables

static int LENGTH_COUNTER_LUT[32] = { 10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30 };
static uint8_t TRI_SEQUENCER[32] = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
static uint32_t NOISE_PERIOD_LUT[16] = { 4,8,16,32,64,96,128,160,202,254,380,508,762,1016,2034,4068 };
static float LOW_PASS_WEIGHTS[AUDIO_SAMPLES];
static float PULSE_LUT[31];
static float TND_LUT[203];

float sinc(float x)
{
	if (x != 0.0f)
	{
		float xpi = x * M_PI;
		return sinf(xpi) / xpi;
	}

	return 1.0f;
}

void AudioInit()
{
	// Low pass filter:	https://rjeschke.tumblr.com/post/8382596050/fir-filters-in-practice
	const float cutoff_freq = 22050.0f;
	const float sample_rate = SAMPLE_RATE;

	const float cutoff = cutoff_freq / sample_rate;
	const float factor = 2.0f * cutoff;
	const int half = AUDIO_SAMPLES / 2;
	for (int i = 0; i < AUDIO_SAMPLES; i++)
	{
		LOW_PASS_WEIGHTS[i] = factor * sinc(factor * (i - half));
	}

	// Mixer LUT's
	PULSE_LUT[0] = 0.0f;
	for (int i = 1; i < 31; i++)
	{
		PULSE_LUT[i] = 95.52f / (8128.0f / i + 100.0f);
	}

	TND_LUT[0] = 0.0f;
	for (int i = 1; i < 203; i++)
	{
		TND_LUT[i] = 163.67f / (24329.0f / i + 100.0f);
	}

}

float apu_filtered_sample(State2A03* apu)
{
	float ret = 0.0f;
	for (int i = 0; i < AUDIO_SAMPLES; i++)
	{
		ret += LOW_PASS_WEIGHTS[i] * apu->audio_samples[i];
	}
	return ret;
}

bool is_mute(State2A03* apu, int channel)
{
	if (channel == SQ1)
	{
		int16_t delta_p = apu->SQ1_timer.period >> apu->SQ1_SWEEP.bits.S;
		if (apu->SQ1_SWEEP.bits.N)
		{
			delta_p = -delta_p;
		}
		apu->SQ1_sweep.target = (int32_t)apu->SQ1_timer.period + delta_p;

		if (apu->SQ1_sweep.target > 0x07FF || apu->SQ1_timer.period < 8)
		{
			return true;
		}
	}
	else if (channel == SQ2)
	{
		int16_t delta_p = apu->SQ2_timer.period >> apu->SQ2_SWEEP.bits.S;
		if (apu->SQ2_SWEEP.bits.N)
		{
			delta_p = -delta_p;
		}
		apu->SQ2_sweep.target = (int32_t)apu->SQ2_timer.period + delta_p;

		if (apu->SQ2_sweep.target > 0x07FF || apu->SQ2_timer.period < 8)
		{
			return true;
		}
	}
	return false;
}

void quarter_frame(State2A03* apu)
{
	// Pulse 1
	if (apu->SQ1_envelope.start_flag)
	{
		apu->SQ1_envelope.start_flag = false;
		apu->SQ1_envelope.decay = 0x0F;
		reload_divider(&apu->SQ1_envelope.div);
	}
	else if (clock_divider(&apu->SQ1_envelope.div))
	{
		if (apu->SQ1_envelope.decay > 0)
		{
			apu->SQ1_envelope.decay--;
		}
		else if (apu->SQ1_VOL.bits.L)
		{
			apu->SQ1_envelope.decay = 0x0F;
		}
	}

	// Pulse 2
	if (apu->SQ2_envelope.start_flag)
	{
		apu->SQ2_envelope.start_flag = false;
		apu->SQ2_envelope.decay = 0x0F;
		reload_divider(&apu->SQ2_envelope.div);
	}
	else if (clock_divider(&apu->SQ2_envelope.div))
	{
		if (apu->SQ2_envelope.decay > 0)
		{
			apu->SQ2_envelope.decay--;
		}
		else if (apu->SQ2_VOL.bits.L)
		{
			apu->SQ2_envelope.decay = 0x0F;
		}
	}

	// Triangle
	if (apu->TRI_linear_reload_flag)
	{
		apu->TRI_linear_counter = apu->TRI_LINEAR.bits.R;
	}
	else if (apu->TRI_linear_counter > 0)
	{
		apu->TRI_linear_counter--;
	}

	if (!apu->TRI_LINEAR.bits.C)
	{
		apu->TRI_linear_reload_flag = false;
	}

	// Noise
	if (apu->NOISE_envelope.start_flag)
	{
		apu->NOISE_envelope.start_flag = false;
		apu->NOISE_envelope.decay = 0x0F;
		reload_divider(&apu->NOISE_envelope.div);
	}
	else if (clock_divider(&apu->NOISE_envelope.div))
	{
		if (apu->NOISE_envelope.decay > 0)
		{
			apu->NOISE_envelope.decay--;
		}
		else if (apu->NOISE_PERIOD.bits.L)
		{
			apu->NOISE_envelope.decay = 0x0F;
		}
	}
}

void half_frame(State2A03* apu)
{
	// Pulse 1
	if (apu->SQ1_length_counter > 0 && !apu->SQ1_VOL.bits.L)
	{
		apu->SQ1_length_counter--;
	}

	if (clock_divider(&apu->SQ1_sweep.div) && !is_mute(apu, SQ1) && apu->SQ1_SWEEP.bits.E)
	{
		apu->SQ1_timer.period = apu->SQ1_sweep.target;
	}
	if (apu->SQ1_sweep.reload_flag)
	{
		apu->SQ1_sweep.reload_flag = false;
		reload_divider(&apu->SQ1_sweep.div);
	}

	// Pulse 2
	if (apu->SQ2_length_counter > 0 && !apu->SQ2_VOL.bits.L)
	{
		apu->SQ2_length_counter--;
	}

	if (clock_divider(&apu->SQ2_sweep.div) && !is_mute(apu, SQ2) && apu->SQ2_SWEEP.bits.E)
	{
		apu->SQ2_timer.period = apu->SQ2_sweep.target;
	}
	if (apu->SQ2_sweep.reload_flag)
	{
		apu->SQ2_sweep.reload_flag = false;
		reload_divider(&apu->SQ2_sweep.div);
	}

	// Triangle
	if (apu->TRI_length_counter > 0 && !apu->TRI_LINEAR.bits.C)
	{
		apu->TRI_length_counter--;
	}

	// Noise
	if (apu->NOISE_length_counter > 0 && !apu->NOISE_VOL.bits.L)
	{
		apu->NOISE_length_counter--;
	}
}

void clock_frame_counter(State2A03* apu)
{
	apu->frame_count++;

	if (apu->FRAME_COUNTER.flags.M)
	{
		if (apu->frame_count == 7456 || apu->frame_count == 18640)
		{
			half_frame(apu);
			quarter_frame(apu);
		}
		if (apu->frame_count == 3728 || apu->frame_count == 18640)
		{
			quarter_frame(apu);
		}
		if (apu->frame_count == 18641)
		{
			apu->frame_count = 0;
		}
	}
	else
	{
		if (apu->frame_count == 7456 || apu->frame_count == 14914)
		{
			half_frame(apu);
			quarter_frame(apu);
		}
		if (apu->frame_count == 3728 || apu->frame_count == 11185)
		{
			quarter_frame(apu);
		}
		if (apu->frame_count == 14915)
		{
			if (!apu->FRAME_COUNTER.flags.I)
			{
				IRQ_Set(apu->cpu, 1);
				apu->IRQ_flag = true;
			}
			apu->frame_count = 0;
		}
	}
}

void clock_2A03(State2A03* apu)
{
	apu->total_cycles++;

	if (apu->total_cycles % 6 == 0)
	{
		apu->apu_cycles++;
		clock_frame_counter(apu);

		// Pulse 1
		if (clock_divider(&apu->SQ1_timer))
		{
			bool seq_out = (apu->SQ1_sequencer & apu->SQ1_sequence_sel) > 0;
			apu->SQ1_sequence_sel = (apu->SQ1_sequence_sel >> 7) | (apu->SQ1_sequence_sel << 1); // Rotate left

			if (seq_out && apu->SQ1_length_counter != 0 && !is_mute(apu, SQ1))
			{
				apu->channel_out.pulse1 = apu->SQ1_VOL.bits.C ? apu->SQ1_VOL.bits.EV : apu->SQ1_envelope.decay;
			}
			else
			{
				apu->channel_out.pulse1 = 0;
			}
		}

		// Pulse 2
		if (clock_divider(&apu->SQ2_timer))
		{
			bool seq_out = (apu->SQ2_sequencer & apu->SQ2_sequence_sel) > 0;
			apu->SQ2_sequence_sel = (apu->SQ2_sequence_sel >> 7) | (apu->SQ2_sequence_sel << 1); // Rotate left

			if (seq_out && apu->SQ2_length_counter != 0 && !is_mute(apu, SQ2))
			{
				apu->channel_out.pulse2 = apu->SQ2_VOL.bits.C ? apu->SQ2_VOL.bits.EV : apu->SQ2_envelope.decay;
			}
			else
			{
				apu->channel_out.pulse2 = 0;
			}
		}

		// Noise
		if (clock_divider(&apu->NOISE_timer))
		{
			uint16_t feedback = (apu->NOISE_LFSR & 1) ^ ((apu->NOISE_LFSR & (apu->NOISE_PERIOD.bits.L ? 1 << 6 : 2)) > 0);
			apu->NOISE_LFSR = (apu->NOISE_LFSR >> 1) | (feedback << 14);

			if (!(apu->NOISE_LFSR & 1) && apu->NOISE_length_counter != 0)
			{
				apu->channel_out.noise = apu->NOISE_VOL.bits.C ? apu->NOISE_VOL.bits.V : apu->NOISE_envelope.decay;
			}
			else
			{
				apu->channel_out.noise = 0;
			}
		}
	}

	// Triangle timer clocked at twice the freqency as the pulse channel
	if (apu->total_cycles % 3 == 0)
	{
		if (clock_divider(&apu->TRI_timer) && apu->TRI_length_counter != 0 && apu->TRI_linear_counter != 0)
		{
			apu->TRI_sequence_index = (apu->TRI_sequence_index + 1) % 32;
			apu->channel_out.triangle = TRI_SEQUENCER[apu->TRI_sequence_index];
		}

		// Shift right
		for (int i = AUDIO_SAMPLES - 1; i > 0; i--)
		{
			apu->audio_samples[i] = apu->audio_samples[i - 1];
		}

		int pulse_index = 0;
		if (apu->channel_enable & CHANNEL_SQ1)
			pulse_index += apu->channel_out.pulse1;
		if (apu->channel_enable & CHANNEL_SQ2)
			pulse_index += apu->channel_out.pulse2;

		int tnd_index = 0;
		if (apu->channel_enable & CHANNEL_TRI)
			tnd_index += 3 * apu->channel_out.triangle;
		if (apu->channel_enable & CHANNEL_NOISE)
			tnd_index += 2 * apu->channel_out.noise;

		apu->audio_samples[0] = PULSE_LUT[pulse_index] + TND_LUT[tnd_index];
	}
}

void reset_2A03(State2A03* apu)
{
	apu->STATUS.reg = 0x00;
	apu->FRAME_COUNTER.reg = 0x00;
	apu->FRAME_COUNTER.flags.I = 1; // Inhibit interrupts

	apu->frame_count = 0;
	apu->IRQ_flag = false;

	apu->total_cycles = 0;
	apu->apu_cycles = 0;

	apu->SQ1_sequence_sel = 0x80;
	apu->SQ2_sequence_sel = 0x80;

	apu->TRI_sequence_index = 0;
}

void power_on_2A03(State2A03* apu)
{
	apu->channel_enable = ~0x0; // Enable all channels
	apu->NOISE_LFSR = 1;
}

void apu_write(State2A03* apu, uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000: // SQ1_VOL: Duty and volume for pulse channel 1
		apu->SQ1_VOL.reg = data;

		switch (apu->SQ1_VOL.bits.D)
		{
		case 0: apu->SQ1_sequencer = 0b00000001; break;
		case 1: apu->SQ1_sequencer = 0b00000011; break;
		case 2: apu->SQ1_sequencer = 0b00001111; break;
		case 3: apu->SQ1_sequencer = 0b11111100; break;
		}
		apu->SQ1_envelope.div.period = apu->SQ1_VOL.bits.EV;
		break;
	case 0x4001: // SQ1_SWEEP: Sweep control for pulse channel 1
		apu->SQ1_SWEEP.reg = data;

		apu->SQ1_sweep.div.period = apu->SQ1_SWEEP.bits.P;
		apu->SQ1_sweep.reload_flag = true;
		break;
	case 0x4002: // SQ1_LO: Low byte period for pulse channel 1
		apu->SQ1_LO = data;

		apu->SQ1_timer.period = ((uint32_t)apu->SQ1_HI.bits.H << 8) | (uint32_t)apu->SQ1_LO;
		break;
	case 0x4003: // SQ1_HI: High byte period for pulse channel 1
		apu->SQ1_HI.reg = data;

		apu->SQ1_envelope.start_flag = true;
		apu->SQ1_timer.period = ((uint32_t)apu->SQ1_HI.bits.H << 8) | (uint32_t)apu->SQ1_LO;
		apu->SQ1_sequence_sel = 0x80;
		if (apu->STATUS.flags.P1)
		{
			apu->SQ1_length_counter = LENGTH_COUNTER_LUT[apu->SQ1_HI.bits.L];
		}
		break;
	case 0x4004: // SQ2_VOL: Duty and volume for pulse channel 2
		apu->SQ2_VOL.reg = data;

		switch (apu->SQ2_VOL.bits.D)
		{
		case 0: apu->SQ2_sequencer = 0b00000001; break;
		case 1: apu->SQ2_sequencer = 0b00000011; break;
		case 2: apu->SQ2_sequencer = 0b00001111; break;
		case 3: apu->SQ2_sequencer = 0b11111100; break;
		}
		apu->SQ2_envelope.div.period = apu->SQ2_VOL.bits.EV;
		break;
	case 0x4005: // SQ2_SWEEP: Sweep control for pulse channel 2
		apu->SQ2_SWEEP.reg = data;

		apu->SQ2_sweep.div.period = apu->SQ2_SWEEP.bits.P;
		apu->SQ2_sweep.reload_flag = true;
		break;
	case 0x4006: // SQ2_LO: Low byte period for pulse channel 2
		apu->SQ2_LO = data;

		apu->SQ2_timer.period = ((uint32_t)apu->SQ2_HI.bits.H << 8) | (uint32_t)apu->SQ2_LO;
		break;
	case 0x4007: // SQ2_HI: High byte period for pulse channel 2
		apu->SQ2_HI.reg = data;

		apu->SQ2_envelope.start_flag = true;
		apu->SQ2_timer.period = ((uint32_t)apu->SQ2_HI.bits.H << 8) | (uint32_t)apu->SQ2_LO;
		apu->SQ2_sequence_sel = 0x80;
		if (apu->STATUS.flags.P2)
		{
			apu->SQ2_length_counter = LENGTH_COUNTER_LUT[apu->SQ2_HI.bits.L];
		}
		break;
	case 0x4008: // TRI_LINEAR: Triangle wave linear counter
		apu->TRI_LINEAR.reg = data;
		break;
	case 0x4009: // Unused
		break;
	case 0x400A: // TRI_LOW: Low byte period for triangle channel
		apu->TRI_LOW = data;

		apu->TRI_timer.period = ((uint32_t)apu->TRI_HIGH.bits.H << 8) | (uint32_t)apu->TRI_LOW;
		break;
	case 0x400B: // TRI_HIGH: High byte period for triangle channel
		apu->TRI_HIGH.reg = data;

		apu->TRI_timer.period = ((uint32_t)apu->TRI_HIGH.bits.H << 8) | (uint32_t)apu->TRI_LOW;

		// Set linear counter reload flag
		apu->TRI_linear_reload_flag = true;

		if (apu->STATUS.flags.T)
		{
			apu->TRI_length_counter = LENGTH_COUNTER_LUT[apu->TRI_HIGH.bits.l];
		}
		break;
	case 0x400C: // NOISE_VOL: Volume for noise channel
		apu->NOISE_VOL.reg = data;

		apu->NOISE_envelope.div.period = apu->NOISE_VOL.bits.V;
		break;
	case 0x400D: // Unused
		break;
	case 0x400E: // NOISE_PERIOD: Period and waveform shape for noise channel
		apu->NOISE_PERIOD.reg = data;

		apu->NOISE_timer.period = NOISE_PERIOD_LUT[apu->NOISE_PERIOD.bits.P];
		break;
	case 0x400F: // NOISE_COUNT: Length counter value for noise channel
		apu->NOISE_COUNT.reg = data;

		apu->NOISE_envelope.start_flag = true;

		if (apu->STATUS.flags.N)
		{
			apu->NOISE_length_counter = LENGTH_COUNTER_LUT[apu->NOISE_COUNT.bits.L];
		}

		break;
	case 0x4010: // DMC_FREQ: Play mode and frequency for DMC samples
		apu->DMC_FREQ.reg = (data & 0xCF);
		break;
	case 0x4011: // DMC_RAW: 7-bit DAC
		break;
	case 0x4012: // DMC_START
		break;
	case 0x4013: // DMC_LEN
		break;
				 // Skip 0x4014 (OAMDMA)

	case 0x4015: // STATUS: Sound channel enable and status
		apu->STATUS.reg = data & 0x1F;

		if (!apu->STATUS.flags.P1)
		{
			apu->SQ1_length_counter = 0;
		}
		if (!apu->STATUS.flags.P2)
		{
			apu->SQ2_length_counter = 0;
		}
		if (!apu->STATUS.flags.T)
		{
			apu->TRI_length_counter = 0;
		}
		if (!apu->STATUS.flags.N)
		{
			apu->NOISE_length_counter = 0;
		}
		break;
				 // Skip 0x4016 (JOY1 data)

	case 0x4017: // Frame counter control
		apu->FRAME_COUNTER.reg = data;
		if (apu->FRAME_COUNTER.flags.I)
		{
			IRQ_Clear(apu->cpu, 1);
			apu->IRQ_flag = false;
		}
		apu->frame_count = 0;
		if (apu->FRAME_COUNTER.flags.M)
		{
			quarter_frame(apu);
			half_frame(apu);
		}
		break;
	}
}

uint8_t apu_read(State2A03* apu, uint16_t addr)
{
	if (addr == 0x4015)
	{
		IRQ_Clear(apu->cpu, 1);

		union
		{
			struct
			{
				uint8_t SQ1 : 1;
				uint8_t SQ2 : 1;
				uint8_t T : 1;
				uint8_t N : 1;
				uint8_t D : 1;
				uint8_t Unused : 1;
				uint8_t F : 1; // Frame interrupt flag
				uint8_t I : 1; // DMC bytes remaining
			} bits;
			uint8_t reg;
		} ret;
		ret.reg = 0;
		ret.bits.F = apu->IRQ_flag;
		apu->IRQ_flag = false;
		if (apu->SQ1_length_counter > 0)
		{
			ret.bits.SQ1 = 1;
		}
		if (apu->SQ2_length_counter > 0)
		{
			ret.bits.SQ2 = 1;
		}
		if (apu->TRI_length_counter > 0)
		{
			ret.bits.T = 1;
		}
		if (apu->NOISE_length_counter > 0)
		{
			ret.bits.N = 1;
		}
		return ret.reg;
	}
	return 0;
}

void apu_channel_set(State2A03* apu, int channel, bool en)
{
	if (en)
	{
		apu->channel_enable |= channel;
	}
	else
	{
		apu->channel_enable &= ~channel;
	}
}

// Divider Implementation

bool clock_divider(divider* div)
{
	if (div->counter == 0)
	{
		div->counter = div->period;
		return true;
	}

	div->counter--;
	return false;
}

void reload_divider(divider* div)
{
	div->counter = div->period;
}