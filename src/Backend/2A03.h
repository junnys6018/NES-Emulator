#ifndef _2A03_H
#define _2A03_H
#include <stdint.h>
#include <stdbool.h>

// Forward declaration to avoid circular dependency
struct State6502;
void IRQ_Set(struct State6502* cpu, int index);
void IRQ_Clear(struct State6502* cpu, int index);

// "2A03" APU implementation
typedef struct
{
	// Registers

	// Access: write only, accessed from $4010
	union
	{
		struct
		{
			uint8_t Frequency : 4; // Pulse 1 enable
			uint8_t Unused : 2; // Pulse 2 enable
			uint8_t L : 1; // Loop
			uint8_t I : 1; // IRQ Enable
		} flags;
		uint8_t reg;
	} DMC_FREQ;

	// Access: read/write, accessed from $4015
	union
	{
		struct
		{
			uint8_t P1 : 1; // Pulse 1 enable
			uint8_t P2 : 1; // Pulse 2 enable
			uint8_t T  : 1; // Triangle enable
			uint8_t N  : 1; // Noise enable
			uint8_t D  : 1; // DMC enable
			uint8_t Unused : 3;
		} flags;
		uint8_t reg;
	} STATUS;

	// Access: write only, accessed from $4017
	union
	{
		struct
		{
			uint8_t Unused : 6; // in reality these bits are used to read data from JOY2
			uint8_t I : 1; // Interrupt inhibit flag. If set, the frame interrupt flag is cleared, otherwise it is unaffected.
			uint8_t M : 1; // Sequencer mode: 0 selects 4-step sequence, 1 selects 5-step sequence
		} flags;
		uint8_t reg;
	} FRAME_COUNTER;

	// Internal stuff

	// Frame Counter
	uint8_t step;
	uint32_t frame_count;

	// Counts the number of master clock cycles ie PPU cycles
	uint64_t total_cycles;

	// Counts the number of APU cycles (6 PPU cycles to 1 APU cycle
	uint64_t apu_cycles;

	struct State6502* cpu;
} State2A03;

void clock_2A03(State2A03* apu);
void reset_2A03(State2A03* apu);
void power_on_2A03(State2A03* apu);
void apu_write(State2A03* apu, uint16_t addr, uint8_t data);
uint8_t apu_read(State2A03* apu, uint16_t addr);

// Common APU compnents
typedef struct
{
	uint32_t period; // actual period is one clock cycle more than given period
	uint32_t counter;
} divider;

bool clock_divider(divider* div);
void reload_divider(divider* div);
#endif