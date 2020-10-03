#ifndef _6502_H
#define _6502_H
#include "6502_Bus.h"

typedef enum 
{
	NO_INTERRUPT = 0,
	IRQ_SIGNAL = 1,
	NMI_SIGNAL = 2
} InterruptSignal;

typedef struct State6502
{
	uint8_t A; // Accumilator register 
	uint8_t X; // X and Y index registers
	uint8_t Y;
	uint16_t PC; // Program counter
	uint8_t SP; // Stack pointer

	union
	{
		struct
		{
			uint8_t C : 1;		// Carry
			uint8_t Z : 1;		// Zero
			uint8_t I : 1;		// Interrupt disable
			uint8_t D : 1;		// Decimal
			uint8_t unused : 2;
			uint8_t V : 1;		// Overflow
			uint8_t N : 1;		// Negative
		} flags;
		uint8_t reg;
	} status;

	Bus6502* bus;

	// DMA Port, Access: write, accessed from $4014
	uint8_t OAMDMA;
	int dma_transfer_cycles;

	// Helper members
	uint8_t operand; // Data fetched for the operand of an instruction
	uint16_t addr; // Address of the data fetched, only set for zero page, absolute addressing and indirect X, Y 
	uint32_t remaining; // Remaining cycles in the instruction

	/* 
	 * Data fetched from an absolute indirect addressing mode operation, 
	 * only the JMP instruction uses this addressing mode. Unlike other addressing modes
	 * 2 bytes are fetched.
	 */
	uint16_t indirect_fetch;
	
	uint64_t total_cycles;

	int interrupt; // Btiwise or of InterruptSignal enums, represents the internal latches that signify an interrupt

} State6502;

// Clock returns the number of cycles until the instruction is completed
int clock_6502(State6502* cpu);
void reset_6502(State6502* cpu);
void power_on_6502(State6502* cpu);
void NMI(State6502* cpu);
void IRQ(State6502* cpu);

char* dissassemble(State6502* cpu, uint16_t addr, int* size);

#endif

