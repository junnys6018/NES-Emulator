#ifndef _6502_H
#define _6502_H
#include "6502Bus.h"

typedef struct State6502
{
	uint8_t A; // Accumilator register
	uint8_t X; // X and Y index registers
	uint8_t Y;
	uint16_t PC; // Program counter
	uint8_t SP;	 // Stack pointer

	union
	{
		struct
		{
			uint8_t C : 1; // Carry
			uint8_t Z : 1; // Zero
			uint8_t I : 1; // Interrupt disable
			uint8_t D : 1; // Decimal
			uint8_t unused : 2;
			uint8_t V : 1; // Overflow
			uint8_t N : 1; // Negative
		} flags;
		uint8_t reg;
	} status;

	Bus6502* bus;

	// DMA Port, Access: write, accessed from $4014
	uint8_t OAMDMA;
	int dma_transfer_cycles;

	// APU can stall the cpu when its reading memory for DMC
	int apu_stall_cycles;

	// Helper members
	uint8_t operand;	// Data fetched for the operand of an instruction
	uint16_t addr;		// Address of the data fetched, only set for zero page, absolute addressing and indirect X, Y
	uint32_t remaining; // Remaining cycles in the instruction

	/* 
	 * Data fetched from an absolute indirect addressing mode operation, 
	 * only the JMP instruction uses this addressing mode. Unlike other addressing modes
	 * 2 bytes are fetched.
	 */
	uint16_t indirect_fetch;

	uint64_t total_cycles;

	// Internal latch that is set on the falling edge of the interrupt line
	bool nmi_interrupt;

	// IRQ is level trigged, active low
	// 8 Devices on the nes are capable of pulling the IRQ line low
	// It is the responsibilty of the IRQ handler to acknowledge the interrupt so that
	// the source stops pulling the line low. Each bit of the variable below represents whether
	// a particular source is pulling the IRQ line low (1: pulling low; 0: not pulling low)
	// Each bit corresponds to a specific device listed below
	//  ---- ----
	//  7654 3210
	//  |||| ||||
	//  |||| |||+-- APU DMC finish
	//  |||| ||+--- APU Frame counter
	//  |||| |+---- MMC3
	//  |||| +----- MMC5
	//  |||+------- VRC4/6/7
	//  ||+-------- FME-7
	//  |+--------- Namco 163
	//  +---------- FDS (and used in IRQ test code)
	uint8_t irq_interrupt;

} State6502;

// Clock returns the number of cycles until the instruction is completed
int clock_6502(State6502* cpu);
void reset_6502(State6502* cpu);
void power_on_6502(State6502* cpu);
void nmi(State6502* cpu);
void irq_set(State6502* cpu, int index);
void irq_clear(State6502* cpu, int index);

void dissassemble(State6502* cpu, uint16_t addr, int* size, char line[128]);

#endif
