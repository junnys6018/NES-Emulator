#include "2A03.h"
#include <stdio.h>

void clock_2A03(State2A03* apu)
{
	apu->total_cycles++;
	if (apu->total_cycles % 6 == 0)
	{
		apu->apu_cycles++;
		apu->frame_count++;
		if (apu->FRAME_COUNTER.flags.M)
		{
			if (apu->frame_count == 18641)
			{
				apu->frame_count = 0;
			}
		}
		else
		{
			if (apu->frame_count == 14915)
			{
				if (!apu->FRAME_COUNTER.flags.I)
				{
					printf("IRQ\n");
					IRQ(apu->cpu);
				}
				apu->frame_count = 0;
			}
		}
	}
}

void reset_2A03(State2A03* apu)
{
	apu->STATUS.reg = 0x00;


	apu->total_cycles = 0;
	apu->apu_cycles = 0;
}

void power_on_2A03(State2A03* apu)
{
	apu->STATUS.reg = 0x00;
}

void apu_write(State2A03* apu, uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000: // SQ1_VOL: Duty and volume for pulse channel 1
		break;
	case 0x4001: // SQ1_SWEEP: Sweep control for pulse channel 1
		break;
	case 0x4002: // SQ1_LO: Low byte period for pulse channel 1
		break;
	case 0x4003: // SQ1_HI: High byte period for pulse channel 1
		break;
	case 0x4004: // SQ2_VOL: Duty and volume for pulse channel 2
		break;
	case 0x4005: // SQ2_SWEEP: Sweep control for pulse channel 2
		break;
	case 0x4006: // SQ2_LO: Low byte period for pulse channel 2
		break;
	case 0x4007: // SQ2_HI: High byte period for pulse channel 2
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
		break;
				 // Skip 0x4016 (JOY1 data)

	case 0x4017: // Frame counter control
		apu->FRAME_COUNTER.reg = data & 0xC0;
		apu->frame_count = 0;
		// printf("FRAME_COUNTER write: M: %i; I: %i\n", apu->FRAME_COUNTER.flags.M, apu->FRAME_COUNTER.flags.I);
		break;
	}
}

uint8_t apu_read(State2A03* apu, uint16_t addr)
{
	if (addr == 0x4015)
	{
		// printf("Read\n");
	}
	return 0;
}


// DIVIDER IMPLEMENTATION

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