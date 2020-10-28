#include "2A03.h"
#include "nes.h" // For sample rate
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

// Reminder: sweep mute 

static LENGTH_COUNTER_LUT[32] = { 10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30 };


// Filtering
void init_low_pass(filter_data* filter, uint32_t cutoff_freq, uint32_t sample_rate)
{
	filter->last_filter = 0.0f;
	filter->last_sample = 0.0f;

	float RC = 1.0f / (2.0f * M_PI * (float)cutoff_freq);
	float dt = 1.0f / (float)sample_rate;

	filter->factor = dt / (dt + RC);
}

void init_high_pass(filter_data* filter, uint32_t cutoff_freq, uint32_t sample_rate)
{
	filter->last_filter = 0.0f;
	filter->last_sample = 0.0f;

	float RC = 1.0f / (2.0f * M_PI * (float)cutoff_freq);
	float dt = 1.0f / (float)sample_rate;

	filter->factor = RC / (dt + RC);
}

float run_high_pass(filter_data* filter, float sample)
{
	float val = filter->factor * filter->last_filter + filter->factor * (sample - filter->last_sample);
	filter->last_filter = val;
	filter->last_sample = sample;
	return val;
}

float run_low_pass(filter_data* filter, float sample)
{
	float val = filter->factor * sample + (1 - filter->factor) * filter->last_filter;
	filter->last_filter = val;
	filter->last_sample = sample;
	return val;
}

bool is_mute(State2A03* apu, int channel)
{
	if (channel == 0)
	{
		int16_t delta_p = apu->SQ1_timer.period >> apu->SQ1_SWEEP.bits.S;
		if (apu->SQ1_SWEEP.bits.N)
		{
			delta_p = -delta_p;
		}
		uint32_t target = (int32_t)apu->SQ1_timer.period + delta_p;

		if (target > 0x07FF)
		{
			return true;
		}
	}
	else
	{
		int16_t delta_p = apu->SQ2_timer.period >> apu->SQ2_SWEEP.bits.S;
		if (apu->SQ2_SWEEP.bits.N)
		{
			delta_p = -delta_p;
		}
		uint32_t target = (int32_t)apu->SQ2_timer.period + delta_p;

		if (target > 0x07FF)
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

	if (apu->SQ1_VOL.bits.C)
	{
		apu->SQ1_envelope.output = apu->SQ1_VOL.bits.EV;
	}
	else
	{
		apu->SQ1_envelope.output = apu->SQ1_envelope.decay;
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

	if (apu->SQ2_VOL.bits.C)
	{
		apu->SQ2_envelope.output = apu->SQ2_VOL.bits.EV;
	}
	else
	{
		apu->SQ2_envelope.output = apu->SQ2_envelope.decay;
	}
}

void half_frame(State2A03* apu)
{
	// Pulse 1
	if (apu->SQ1_length_counter != 0 && !apu->SQ1_VOL.bits.L)
	{
		apu->SQ1_length_counter--;
	}
	if (clock_divider(&apu->SQ1_sweep.div) && !is_mute(apu, 0) && apu->SQ1_timer.period >= 8)
	{
		if (apu->SQ1_SWEEP.bits.E)
		{
			int16_t delta_p = apu->SQ1_timer.period >> apu->SQ1_SWEEP.bits.S;
			if (apu->SQ1_SWEEP.bits.N)
			{
				delta_p = -delta_p;
			}
			uint32_t target = (int32_t)apu->SQ1_timer.period + delta_p;

			apu->SQ1_timer.period = target;
		}
	}
	if (apu->SQ1_sweep.reload_flag)
	{
		apu->SQ1_sweep.reload_flag = false;
		reload_divider(&apu->SQ1_sweep.div);
	}

	// Pulse 2
	if (apu->SQ2_length_counter != 0 && !apu->SQ2_VOL.bits.L)
	{
		apu->SQ2_length_counter--;
	}
	if (clock_divider(&apu->SQ2_sweep.div))
	{
		if (apu->SQ2_SWEEP.bits.E && !is_mute(apu, 1) && apu->SQ2_timer.period >= 8)
		{
			int16_t delta_p = apu->SQ2_timer.period >> apu->SQ2_SWEEP.bits.S;
			if (apu->SQ2_SWEEP.bits.N)
			{
				delta_p = -delta_p;
			}
			uint32_t target = (int32_t)apu->SQ2_timer.period + delta_p;

			apu->SQ2_timer.period = target;
		}
	}
	if (apu->SQ2_sweep.reload_flag)
	{
		apu->SQ2_sweep.reload_flag = false;
		reload_divider(&apu->SQ2_sweep.div);
	}
}

void clock_frame_counter(State2A03* apu)
{
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
		apu->frame_count++;

		if (clock_divider(&apu->SQ1_timer))
		{
			bool seq_out = (apu->SQ1_sequencer & 0x80) > 0;
			apu->SQ1_sequencer = (apu->SQ1_sequencer >> 7) | (apu->SQ1_sequencer << 1); // Rotate left
			if (seq_out && apu->SQ1_length_counter != 0 && apu->SQ1_timer.period >= 8 && !is_mute(apu, 0))
			{
				apu->channel_out.pulse1 = apu->SQ1_envelope.output;
			}
			else
			{
				apu->channel_out.pulse1 = 0;
			}
		}

		if (clock_divider(&apu->SQ2_timer))
		{
			bool seq_out = (apu->SQ2_sequencer & 0x80) > 0;
			apu->SQ2_sequencer = (apu->SQ2_sequencer >> 7) | (apu->SQ2_sequencer << 1); // Rotate left
			if (seq_out && apu->SQ2_length_counter != 0 && apu->SQ2_timer.period >= 8 && !is_mute(apu, 1))
			{
				apu->channel_out.pulse2 = apu->SQ2_envelope.output;
			}
			else
			{
				apu->channel_out.pulse2 = 0;
			}
		}

		clock_frame_counter(apu);

		apu->audio_sample = 0.00752f * (float)(apu->channel_out.pulse1 + apu->channel_out.pulse2);
	}
}

void reset_2A03(State2A03* apu)
{
	apu->STATUS.reg = 0x00;
	apu->FRAME_COUNTER.reg = 0x00;
	apu->FRAME_COUNTER.flags.I = 1; // Inhibit interrupts

	apu->frame_count = 0;

	apu->total_cycles = 0;
	apu->apu_cycles = 0;
}

void power_on_2A03(State2A03* apu)
{
	// Init filters
	init_high_pass(&apu->filters[0], 20, SAMPLE_RATE);
	init_high_pass(&apu->filters[1], 440, SAMPLE_RATE);
	init_low_pass(&apu->filters[2], 14000, SAMPLE_RATE);
}

void apu_write(State2A03* apu, uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000: // SQ1_VOL: Duty and volume for pulse channel 1
		apu->SQ1_VOL.reg = data;
		switch (apu->SQ1_VOL.bits.D)
		{
		case 0: apu->SQ1_sequencer = 0b01000000; break;
		case 1: apu->SQ1_sequencer = 0b01100000; break;
		case 2: apu->SQ1_sequencer = 0b01111000; break;
		case 3: apu->SQ1_sequencer = 0b10011111; break;
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
		apu->SQ1_envelope.start_flag = true;
		apu->SQ1_HI.reg = data;
		apu->SQ1_timer.period = ((uint32_t)apu->SQ1_HI.bits.H << 8) | (uint32_t)apu->SQ1_LO;

		if (apu->STATUS.flags.P1)
		{
			apu->SQ1_length_counter = LENGTH_COUNTER_LUT[apu->SQ1_HI.bits.L];
		}
		break;
	case 0x4004: // SQ2_VOL: Duty and volume for pulse channel 2
		apu->SQ2_VOL.reg = data;
		switch (apu->SQ2_VOL.bits.D)
		{
		case 0: apu->SQ2_sequencer = 0b01000000; break;
		case 1: apu->SQ2_sequencer = 0b01100000; break;
		case 2: apu->SQ2_sequencer = 0b01111000; break;
		case 3: apu->SQ2_sequencer = 0b10011111; break;
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
		apu->SQ2_envelope.start_flag = true;
		apu->SQ2_HI.reg = data;
		apu->SQ2_timer.period = ((uint32_t)apu->SQ2_HI.bits.H << 8) | (uint32_t)apu->SQ2_LO;

		if (apu->STATUS.flags.P2)
		{
			apu->SQ2_length_counter = LENGTH_COUNTER_LUT[apu->SQ2_HI.bits.L];
		}
		break;
	case 0x4008: // TRI_LINEAR: Triangle wave linear counter
		break;
	case 0x4009: // Unused
		break;
	case 0x400A: // TRI_LOW: Low byte period for triangle channel
		break;
	case 0x400B: // TRI_HIGH: High byte period for triangle channel
		break;
	case 0x400C: // NOISE_VOL: Volume for noise channel
		break;
	case 0x400D: // Unused
		break;
	case 0x400E: // NOISE_LO: Period and waveform shape for noise channel
		break;
	case 0x400F: // NOISE_HI: Length counter value for noise channel
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
		break;
				 // Skip 0x4016 (JOY1 data)

	case 0x4017: // Frame counter control
		apu->FRAME_COUNTER.reg = (data & 0xC0);
		if (apu->FRAME_COUNTER.flags.I)
		{
			IRQ_Clear(apu->cpu, 1);
		}
		apu->frame_count = 0;
		// printf("FRAME_COUNTER write: M: %i; I: %i\n", apu->FRAME_COUNTER.flags.M, apu->FRAME_COUNTER.flags.I);
		break;
	}
}

uint8_t apu_read(State2A03* apu, uint16_t addr)
{
	if (addr == 0x4015)
	{
		union
		{
			struct
			{
				uint8_t SQ1 : 1;
				uint8_t SQ2 : 1;
				uint8_t T : 1;
				uint8_t N : 1;
				uint8_t D : 1;
				uint8_t Unused : 3;
			} bits;
			uint8_t reg;
		} ret;
		ret.reg = 0;
		if (apu->SQ1_length_counter > 0)
		{
			ret.bits.SQ1 = 1;
		}
		if (apu->SQ2_length_counter > 0)
		{
			ret.bits.SQ2 = 1;
		}
		IRQ_Clear(apu->cpu, 1);

		return ret.reg;
	}
	return 0;
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