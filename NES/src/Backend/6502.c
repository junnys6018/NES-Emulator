#include "6502.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> // For printf
#include <stdlib.h> // For malloc

/* 
 * TODO:
 * Interrupt hijacking
 * Illegal Opcodes
 * FIX ROL ASL ROR LSR (merge the matching functions)
 * Decimal Mode
 */

typedef struct
{
	char code[4]; // OP code of the instruction

	// Executes the instruction, changing the state of the cpu, some instructions care if a page boundary was crossed
	// to fetch its data. returns any additional cycles required to complete the instruction if any 
	uint32_t(*operation)(State6502* cpu, bool c);

	// Calculates the location of the operand used in the instruction, call fetch() to load the operand into the CPU
	// Returns true if a page boundary will be crossed to fetch the operand
	bool(*adressing_mode)(State6502* cpu);

	uint32_t cycles; // Number of cycles required for instruction to complete
} Instruction;

static Instruction opcodes[256];

void interrupt_sequence(State6502* cpu, uint16_t interrupt_vector)
{
	// Push PC to stack
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (uint8_t)(cpu->PC >> 8)); // High byte
	cpu->SP--;

	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (uint8_t)(cpu->PC & 0x00FF)); // Low byte
	cpu->SP--;

	// Push status to stack, with bit 5 set and bit 4 clear
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (cpu->status.reg | 1 << 5) & ~(1 << 4));
	cpu->SP--;

	// Set PC to NMI vector 0xFFFA/B
	cpu->PC = cpu_bus_read(cpu->bus, interrupt_vector + 1); // High byte
	cpu->PC = cpu->PC << 8;
	cpu->PC |= cpu_bus_read(cpu->bus, interrupt_vector); // Low byte

	// Set I flag
	cpu->status.flags.I = 1;
}

int clock_6502(State6502* cpu)
{
	static uint32_t remaining = 0;
	cpu->total_cycles++;

	if (remaining == 0)
	{
		if (cpu->interrupt & NMI_SIGNAL)
		{
			remaining = 7; // 7 clock cycles to respond to interrupt
			interrupt_sequence(cpu, 0xFFFA);

			cpu->interrupt &= ~NMI_SIGNAL;
		}
		else if (cpu->interrupt & IRQ_SIGNAL && !cpu->status.flags.I)
		{
			remaining = 7; // 7 clock cycles to respond to interrupt
			interrupt_sequence(cpu, 0xFFFE);

			cpu->interrupt &= ~IRQ_SIGNAL;
		}
		else
		{
			// if IRQ signal was raised, we should ignore it 
			cpu->interrupt &= ~IRQ_SIGNAL;

			uint8_t opcode = cpu_bus_read(cpu->bus, cpu->PC++);
			Instruction inst = opcodes[opcode];

			remaining = inst.cycles;
			remaining--;

			bool b = inst.adressing_mode(cpu);
			remaining += inst.operation(cpu, b);
		}
	}
	else
		remaining--;

	return remaining;
}

void reset_6502(State6502* cpu)
{
	cpu->SP -= 3;
	cpu->status.flags.I = 1;

	cpu_bus_write(cpu->bus, 0x4015, 0); // All channels disabled

	// Set PC to Reset vector 0xFFFC/D
	cpu->PC = cpu_bus_read(cpu->bus, 0xFFFD); // High byte
	cpu->PC = cpu->PC << 8;
	cpu->PC |= cpu_bus_read(cpu->bus, 0xFFFC); // Low byte

	// Reset Cycle count (used in debugging only)
	cpu->total_cycles = 0;
}

void power_on_6502(State6502* cpu)
{
	cpu->status.reg = 0x34; // Interrupt disabled
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	cpu->SP = 0xFD;

	cpu_bus_write(cpu->bus, 0x4015, 0); // All channels disabled
	cpu_bus_write(cpu->bus, 0x4017, 0); // Frame IRQ disabled

	for (uint16_t addr = 0x4000; addr <= 0x400F; addr++)
	{
		cpu_bus_write(cpu->bus, addr, 0);
	}

	for (uint16_t addr = 0x4010; addr <= 0x4013; addr++)
	{
		cpu_bus_write(cpu->bus, addr, 0);
	}

	// Reset Cycle count (used in debugging only)
	cpu->total_cycles = 0;

	cpu->interrupt = NO_INTERRUPT;
}

void NMI(State6502* cpu)
{
	cpu->interrupt |= NMI_SIGNAL;
}

void IRQ(State6502* cpu)
{
	cpu->interrupt |= IRQ_SIGNAL;
}

// 13 Adressing modes
void fetch(State6502* cpu)
{
	cpu->operand = cpu_bus_read(cpu->bus, cpu->addr);
}

// Accumilator addressing
bool ACC(State6502* cpu)
{
	return false;
}

// Immediate addressing
bool IMM(State6502* cpu)
{
	cpu->addr = cpu->PC++;
	return false;
}

// Zero page
bool ZP0(State6502* cpu)
{
	cpu->addr = cpu_bus_read(cpu->bus, cpu->PC++);

	return false;
}

// Zero page with X index
bool ZPX(State6502* cpu)
{
	uint16_t addr_low = cpu_bus_read(cpu->bus, cpu->PC++);
	cpu->addr = (addr_low + cpu->X) & 0x00FF;
	return false;
}

// Zero page with Y index
bool ZPY(State6502* cpu)
{
	uint16_t addr_low = cpu_bus_read(cpu->bus, cpu->PC++);
	cpu->addr = (addr_low + cpu->Y) & 0x00FF;
	return false;
}

// Absolulte
bool ABS(State6502* cpu)
{
	uint16_t addr_low = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = cpu_bus_read(cpu->bus, cpu->PC++);
	cpu->addr = (addr_high << 8) | addr_low;
	return false;
}

// Absolute with X index
bool ABX(State6502* cpu)
{
	uint16_t addr_low = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr = (addr_high << 8) | addr_low;
	cpu->addr = addr + cpu->X;

	return (addr & 0xFF00) != (cpu->addr & 0xFF00);
}

// Absolulte with Y index
bool ABY(State6502* cpu)
{
	uint16_t addr_low = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr = (addr_high << 8) | addr_low;
	cpu->addr = addr + cpu->Y;

	return (addr & 0xFF00) != (cpu->addr & 0xFF00);
}

// Implied
bool IMP(State6502* cpu)
{
	return false;
}

// Relative 
bool REL(State6502* cpu)
{
	cpu->addr = cpu->PC++;
	return false;
}

// Indirect
bool IND(State6502* cpu)
{
	uint16_t addr_low = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr = (addr_high << 8) | addr_low;

	// Read high bits
	// Note: Due to a hardware bug, if the specified address is $xxFF the second byte read will not cross pages
	// ie it will be read from $xx00 instead 255 bytes earlier than expected
	cpu->indirect_fetch = cpu_bus_read(cpu->bus, (addr_high << 8) | ((addr_low + 1) & 0x00FF));
	cpu->indirect_fetch <<= 8;
	cpu->indirect_fetch |= cpu_bus_read(cpu->bus, addr);
	
	/* Hack: only the JMP instruction uses indirect addressing, however it could also be using absolute addressing
	 * If absolute addressing was used JMP would set the program counter to the value of cpu->addr, however
	 * if indirect addressing was used JMP would set the program counter to the value of cpu->indirect_fetch
	 * The problem is that JMP has no way of knowing which addressing mode was used, this is where the hack comes in,
	 * both indirect and absolute addressing will never cross a page boundary, so they should both return false. However 
	 * we set the indirect addressing function to return true so the JMP instruction can tell what addressing mode was used
	 * this has no effect on other instructions as JMP is the only one using indirect addressing
	 */
	return true;
}

// Indirect with X index
bool IZX(State6502* cpu)
{
	uint16_t zp_addr = cpu_bus_read(cpu->bus, cpu->PC++);
	zp_addr = (zp_addr + cpu->X) & 0x00FF;

	uint16_t addr_low = cpu_bus_read(cpu->bus, zp_addr);
	uint16_t addr_high = cpu_bus_read(cpu->bus, (zp_addr + 1) & 0x00FF);
	cpu->addr = (addr_high << 8) | addr_low;

	return false;
}

// Indirect with Y index
bool IZY(State6502* cpu)
{
	uint16_t zp_addr = cpu_bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_low = cpu_bus_read(cpu->bus, zp_addr);
	uint16_t addr_high = cpu_bus_read(cpu->bus, (zp_addr + 1) & 0x00FF); // Read the next zero page address

	uint16_t addr = ((addr_high << 8) | addr_low);
	cpu->addr = addr + cpu->Y;
	return (addr & 0xFF00) != (cpu->addr & 0xFF00);
}

// 56 instructions

// Add with carry
uint32_t ADC(State6502* cpu, bool c)
{
	fetch(cpu);
	uint16_t result = (uint16_t)cpu->A + (uint16_t)cpu->operand + (uint16_t)cpu->status.flags.C;

	// Set status flags
	cpu->status.flags.C = result > 255;
	cpu->status.flags.Z = (result & 0x00FF) == 0;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;
	cpu->status.flags.V = ((~((uint16_t)cpu->A ^ (uint16_t)cpu->operand) & ((uint16_t)cpu->A ^ result)) & 0x0080) == 0x0080;

	// Store result into register A
	cpu->A = result & 0x00FF;

	return c ? 1 : 0;
}

// Logical AND
uint32_t AND(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->A = cpu->A & cpu->operand;

	// Set status flags
	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Arithmetic Shift Left. One version works on the accumilator, the other on memory
uint32_t aASL(State6502* cpu, bool c)
{
	cpu->status.flags.C = (cpu->A & 0x0080) == 0x0080;

	cpu->A = cpu->A << 1;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return 0;
}

uint32_t mASL(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->status.flags.C = (cpu->operand & 0x0080) == 0x0080;

	cpu->operand = cpu->operand << 1;

	cpu->status.flags.Z = cpu->operand == 0;
	cpu->status.flags.N = (cpu->operand & 0x0080) == 0x0080;

	cpu_bus_write(cpu->bus, cpu->addr, cpu->operand);

	return 0;
}

// Branch if Carry Clear
uint32_t BCC(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.C == 0)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Branch if Carry Set
uint32_t BCS(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.C == 1)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Branch if Equal (zero flag set)
uint32_t BEQ(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.Z == 1)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Bit Test
uint32_t BIT(State6502* cpu, bool c)
{
	fetch(cpu);
	uint8_t result = cpu->A & cpu->operand;

	cpu->status.flags.Z = result == 0;
	cpu->status.flags.V = (cpu->operand & (1 << 6)) == (1 << 6);
	cpu->status.flags.N = (cpu->operand & (1 << 7)) == (1 << 7);

	return 0;
}

// Branch if Minus (negative flag set)
uint32_t BMI(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.N == 1)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Branch if not Equal (zero flag clear)
uint32_t BNE(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.Z == 0)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Branch if Positive (negative flag clear)
uint32_t BPL(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.N == 0)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Force Interrupt
uint32_t BRK(State6502* cpu, bool c)
{
	// This is a bug in the 6502, the address after the BRK instruction is not pushed to the stack
	// that address is skipped and the second address after the BRK instruction is pushed instead
	cpu->PC++;

	// Push PC to stack
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (uint8_t)(cpu->PC >> 8)); // High byte
	cpu->SP--;

	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (uint8_t)(cpu->PC & 0x00FF)); // Low byte
	cpu->SP--;

	// Push status to stack, with bits 4 and 5 set
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, cpu->status.reg | 1 << 4 | 1 << 5);
	cpu->SP--;

	// Set PC to BRK vector 0xFFFE/F
	cpu->PC = cpu_bus_read(cpu->bus, 0xFFFF); // High byte
	cpu->PC = cpu->PC << 8;
	cpu->PC |= cpu_bus_read(cpu->bus, 0xFFFE); // Low byte

	// Set I flag
	cpu->status.flags.I = 1;

	return 0;
}

// Branch if Overflow Clear
uint32_t BVC(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.V == 0)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Branch if Overflow Set
uint32_t BVS(State6502* cpu, bool c)
{
	fetch(cpu);
	if (cpu->status.flags.V == 1)
	{
		cpu->PC += (int8_t)cpu->operand;

		return c ? 2 : 1;
	}

	return 0;
}

// Clear Carry Flag
uint32_t CLC(State6502* cpu, bool c)
{
	cpu->status.flags.C = 0;
	return 0;
}

// Clear Decimal Mode
uint32_t CLD(State6502* cpu, bool c)
{
	cpu->status.flags.D = 0;
	return 0;
}

// Clear Interrupt Disable
uint32_t CLI(State6502* cpu, bool c)
{
	cpu->status.flags.I = 0;
	return 0;
}

// Clear Overflow Flag
uint32_t CLV(State6502* cpu, bool c)
{
	cpu->status.flags.V = 0;
	return 0;
}

// Compare A register
uint32_t CMP(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->status.flags.C = (cpu->A >= cpu->operand);
	cpu->status.flags.Z = (cpu->A == cpu->operand);

	uint16_t result = (uint16_t)cpu->A - (uint16_t)cpu->operand;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Compare X Register
uint32_t CPX(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->status.flags.C = (cpu->X >= cpu->operand);
	cpu->status.flags.Z = (cpu->X == cpu->operand);

	uint16_t result = (uint16_t)cpu->X - (uint16_t)cpu->operand;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return 0;
}

// Compare Y Register
uint32_t CPY(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->status.flags.C = (cpu->Y >= cpu->operand);
	cpu->status.flags.Z = (cpu->Y == cpu->operand);

	uint16_t result = (uint16_t)cpu->Y - (uint16_t)cpu->operand;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return 0;
}

// Decrement Memory
uint32_t DEC(State6502* cpu, bool c)
{
	fetch(cpu);
	uint8_t result = cpu->operand - 1;
	cpu_bus_write(cpu->bus, cpu->addr, result);

	cpu->status.flags.Z = result == 0;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;
	return 0;
}

// Decrement X Register
uint32_t DEX(State6502* cpu, bool c)
{
	cpu->X--;

	cpu->status.flags.Z = cpu->X == 0;
	cpu->status.flags.N = (cpu->X & 0x0080) == 0x0080;
	return 0;
}

// Decrement Y Register
uint32_t DEY(State6502* cpu, bool c)
{
	cpu->Y--;

	cpu->status.flags.Z = cpu->Y == 0;
	cpu->status.flags.N = (cpu->Y & 0x0080) == 0x0080;
	return 0;
}

// Exclusive OR
uint32_t EOR(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->A = cpu->A ^ cpu->operand;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Increment Memory
uint32_t INC(State6502* cpu, bool c)
{
	fetch(cpu);
	uint8_t result = cpu->operand + 1;
	cpu_bus_write(cpu->bus, cpu->addr, result);

	cpu->status.flags.Z = result == 0;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;
	return 0;
}

// Increment X Register
uint32_t INX(State6502* cpu, bool c)
{
	cpu->X++;

	cpu->status.flags.Z = cpu->X == 0;
	cpu->status.flags.N = (cpu->X & 0x0080) == 0x0080;
	return 0;
}

// Increment Y Register
uint32_t INY(State6502* cpu, bool c)
{
	cpu->Y++;

	cpu->status.flags.Z = cpu->Y == 0;
	cpu->status.flags.N = (cpu->Y & 0x0080) == 0x0080;
	return 0;
}

// Jump 
uint32_t JMP(State6502* cpu, bool c)
{
	// Indirect addressing used
	if (c)
	{
		cpu->PC = cpu->indirect_fetch;
	}
	else // Absolute addressing used
	{
		cpu->PC = cpu->addr;
	}

	return 0;
}

// Jump to Subroutine
uint32_t JSR(State6502* cpu, bool c)
{
	// Set PC to the last byte of the JSR instruction
	cpu->PC--;

	// Push PC to stack
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (uint8_t)(cpu->PC >> 8)); // High byte
	cpu->SP--;

	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, (uint8_t)(cpu->PC & 0x00FF)); // Low byte
	cpu->SP--;

	cpu->PC = cpu->addr;

	return 0;
}

// Load Accumulator
uint32_t LDA(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->A = cpu->operand;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Load X Register
uint32_t LDX(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->X = cpu->operand;

	cpu->status.flags.Z = cpu->X == 0;
	cpu->status.flags.N = (cpu->X & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Load Y Register
uint32_t LDY(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->Y = cpu->operand;

	cpu->status.flags.Z = cpu->Y == 0;
	cpu->status.flags.N = (cpu->Y & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Logical Shift Right
uint32_t aLSR(State6502* cpu, bool c)
{
	cpu->status.flags.C = cpu->A & 0x0001;

	cpu->A = cpu->A >> 1;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return 0;
}

uint32_t mLSR(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->status.flags.C = cpu->operand & 0x0001;

	cpu->operand = cpu->operand >> 1;

	cpu->status.flags.Z = cpu->operand == 0;
	cpu->status.flags.N = (cpu->operand & 0x0080) == 0x0080;

	cpu_bus_write(cpu->bus, cpu->addr, cpu->operand);

	return 0;
}


// No Operation
uint32_t NOP(State6502* cpu, bool c)
{
	return 0;
}

// Logical Inclusive OR
uint32_t ORA(State6502* cpu, bool c)
{
	fetch(cpu);
	cpu->A = cpu->A | cpu->operand;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Push Accumulator
uint32_t PHA(State6502* cpu, bool c)
{
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, cpu->A);
	cpu->SP--;

	return 0;
}

// Push Processor Status
uint32_t PHP(State6502* cpu, bool c)
{
	// Unused bits 4 and 5 are set in a PHP instruction
	cpu_bus_write(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP, cpu->status.reg | 1 << 4 | 1 << 5);
	cpu->SP--;

	return 0;
}

// Pull Accumulator
uint32_t PLA(State6502* cpu, bool c)
{
	cpu->SP++;
	cpu->A = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return 0;
}

// Pull Processor Status
uint32_t PLP(State6502* cpu, bool c)
{
	cpu->SP++;
	cpu->status.reg = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);

	return 0;
}

// Rotate Left
uint32_t aROL(State6502* cpu, bool c)
{
	uint8_t old_carry = cpu->status.flags.C;
	cpu->status.flags.C = (cpu->A & 0x0080) == 0x0080;

	cpu->A = cpu->A << 1;
	cpu->A |= old_carry;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return 0;
}

uint32_t mROL(State6502* cpu, bool c)
{
	fetch(cpu);
	uint8_t old_carry = cpu->status.flags.C;
	cpu->status.flags.C = (cpu->operand & 0x0080) == 0x0080;

	cpu->operand = cpu->operand << 1;
	cpu->operand |= old_carry;

	cpu->status.flags.Z = cpu->operand == 0;
	cpu->status.flags.N = (cpu->operand & 0x0080) == 0x0080;

	cpu_bus_write(cpu->bus, cpu->addr, cpu->operand);

	return 0;
}

// Rotate Right
uint32_t aROR(State6502* cpu, bool c)
{
	uint8_t old_carry = (uint8_t)cpu->status.flags.C << 7;
	cpu->status.flags.C = cpu->A & 0x0001;

	cpu->A = cpu->A >> 1;
	cpu->A |= old_carry;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return 0;
}

uint32_t mROR(State6502* cpu, bool c)
{
	fetch(cpu);
	uint8_t old_carry = (uint8_t)cpu->status.flags.C << 7;
	cpu->status.flags.C = cpu->operand & 0x0001;

	cpu->operand = cpu->operand >> 1;
	cpu->operand |= old_carry;

	cpu->status.flags.Z = cpu->operand == 0;
	cpu->status.flags.N = (cpu->operand & 0x0080) == 0x0080;

	cpu_bus_write(cpu->bus, cpu->addr, cpu->operand);

	return 0;
}

// Return from Interrupt
uint32_t RTI(State6502* cpu, bool c)
{
	// Pull status from stack
	cpu->SP++;
	cpu->status.reg = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);

	// Pull PC from stack
	cpu->SP++;
	uint16_t PCL = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);
	cpu->SP++;
	uint16_t PCH = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);

	cpu->PC = (PCH << 8) | PCL;

	return 0;
}

// Return from Subroutine
uint32_t RTS(State6502* cpu, bool c)
{
	// Pull PC from stack
	cpu->SP++;
	uint16_t PCL = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);
	cpu->SP++;
	uint16_t PCH = cpu_bus_read(cpu->bus, (uint16_t)0x0100 | (uint16_t)cpu->SP);

	cpu->PC = (PCH << 8) | PCL + 1;

	return 0;
}

// Subtract with Carry
uint32_t SBC(State6502* cpu, bool c)
{
	fetch(cpu);
	uint16_t value = ((uint16_t)cpu->operand) ^ 0x00FF;

	uint16_t temp = (uint16_t)cpu->A + value + (uint16_t)cpu->status.flags.C;
	cpu->status.flags.C = (temp & 0xFF00) != 0;
	cpu->status.flags.Z = (temp & 0x00FF) == 0;
	cpu->status.flags.V = ((temp ^ (uint16_t)cpu->A) & (temp ^ value) & 0x0080) == 0x0080;
	cpu->status.flags.N = (temp & 0x0080) == 0x0080;
	cpu->A = temp & 0x00FF;
	return c ? 1 : 0;
}

// Set Carry Flag
uint32_t SEC(State6502* cpu, bool c)
{
	cpu->status.flags.C = 1;
	return 0;
}

// Set Decimal Flag
uint32_t SED(State6502* cpu, bool c)
{
	cpu->status.flags.D = 1;
	return 0;
}

// Set Interrupt Disable
uint32_t SEI(State6502* cpu, bool c)
{
	cpu->status.flags.I = 1;
	return 0;
}

// Store Accumulator
uint32_t STA(State6502* cpu, bool c)
{
	cpu_bus_write(cpu->bus, cpu->addr, cpu->A);
	return 0;
}

// Store X Register
uint32_t STX(State6502* cpu, bool c)
{
	cpu_bus_write(cpu->bus, cpu->addr, cpu->X);
	return 0;
}

// Store Y Register
uint32_t STY(State6502* cpu, bool c)
{
	cpu_bus_write(cpu->bus, cpu->addr, cpu->Y);
	return 0;
}

// Transfer Accumulator to X
uint32_t TAX(State6502* cpu, bool c)
{
	cpu->X = cpu->A;

	cpu->status.flags.Z = cpu->X == 0;
	cpu->status.flags.N = (cpu->X & 0x0080) == 0x0080;
	return 0;
}

// Transfer Accumulator to Y
uint32_t TAY(State6502* cpu, bool c)
{
	cpu->Y = cpu->A;

	cpu->status.flags.Z = cpu->Y == 0;
	cpu->status.flags.N = (cpu->Y & 0x0080) == 0x0080;
	return 0;
}

// Transfer Stack Pointer to X
uint32_t TSX(State6502* cpu, bool c)
{
	cpu->X = cpu->SP;

	cpu->status.flags.Z = cpu->X == 0;
	cpu->status.flags.N = (cpu->X & 0x0080) == 0x0080;
	return 0;
}

// Transfer X to Accumulator
uint32_t TXA(State6502* cpu, bool c)
{
	cpu->A = cpu->X;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;
	return 0;
}

// Transfer X to Stack Pointer
uint32_t TXS(State6502* cpu, bool c)
{
	cpu->SP = cpu->X;
	return 0;
}

// Transfer Y to Accumulator
uint32_t TYA(State6502* cpu, bool c)
{
	cpu->A = cpu->Y;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;
	return 0;
}

// All illegal opcodes are captured here
uint32_t XXX(State6502* cpu, bool c)
{
	printf("[WARN] Illegal Opcode Used\n");
	return 0;
}

static Instruction opcodes[256] = {
	{"BRK", BRK,IMP,7}, {"ORA", ORA,IZX,6}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,3}, {"ORA", ORA,ZP0,3}, {"ASL",mASL,ZP0,5}, {"???", XXX,IMP,5}, {"PHP", PHP,IMP,3}, {"ORA", ORA,IMM,2}, {"ASL",aASL,ACC,2}, {"???", XXX,IMP,2}, {"???", NOP,IMP,4}, {"ORA", ORA,ABS,4}, {"ASL",mASL,ABS,6}, {"???", XXX,IMP,6},
	{"BPL", BPL,REL,2}, {"ORA", ORA,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,4}, {"ORA", ORA,ZPX,4}, {"ASL",mASL,ZPX,6}, {"???", XXX,IMP,6}, {"CLC", CLC,IMP,2}, {"ORA", ORA,ABY,4}, {"???", NOP,IMP,2}, {"???", XXX,IMP,7}, {"???", NOP,IMP,4}, {"ORA", ORA,ABX,4}, {"ASL",mASL,ABX,7}, {"???", XXX,IMP,7},
	{"JSR", JSR,ABS,6}, {"AND", AND,IZX,6}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"BIT", BIT,ZP0,3}, {"AND", AND,ZP0,3}, {"ROL",mROL,ZP0,5}, {"???", XXX,IMP,5}, {"PLP", PLP,IMP,4}, {"AND", AND,IMM,2}, {"ROL",aROL,ACC,2}, {"???", XXX,IMP,2}, {"BIT", BIT,ABS,4}, {"AND", AND,ABS,4}, {"ROL",mROL,ABS,6}, {"???", XXX,IMP,6},
	{"BMI", BMI,REL,2}, {"AND", AND,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,4}, {"AND", AND,ZPX,4}, {"ROL",mROL,ZPX,6}, {"???", XXX,IMP,6}, {"SEC", SEC,IMP,2}, {"AND", AND,ABY,4}, {"???", NOP,IMP,2}, {"???", XXX,IMP,7}, {"???", NOP,IMP,4}, {"AND", AND,ABX,4}, {"ROL",mROL,ABX,7}, {"???", XXX,IMP,7},
	{"RTI", RTI,IMP,6}, {"EOR", EOR,IZX,6}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,3}, {"EOR", EOR,ZP0,3}, {"LSR",mLSR,ZP0,5}, {"???", XXX,IMP,5}, {"PHA", PHA,IMP,3}, {"EOR", EOR,IMM,2}, {"LSR",aLSR,ACC,2}, {"???", XXX,IMP,2}, {"JMP", JMP,ABS,3}, {"EOR", EOR,ABS,4}, {"LSR",mLSR,ABS,6}, {"???", XXX,IMP,6},
	{"BVC", BVC,REL,2}, {"EOR", EOR,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,4}, {"EOR", EOR,ZPX,4}, {"LSR",mLSR,ZPX,6}, {"???", XXX,IMP,6}, {"CLI", CLI,IMP,2}, {"EOR", EOR,ABY,4}, {"???", NOP,IMP,2}, {"???", XXX,IMP,7}, {"???", NOP,IMP,4}, {"EOR", EOR,ABX,4}, {"LSR",mLSR,ABX,7}, {"???", XXX,IMP,7},
	{"RTS", RTS,IMP,6}, {"ADC", ADC,IZX,6}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,3}, {"ADC", ADC,ZP0,3}, {"ROR",mROR,ZP0,5}, {"???", XXX,IMP,5}, {"PLA", PLA,IMP,4}, {"ADC", ADC,IMM,2}, {"ROR",aROR,ACC,2}, {"???", XXX,IMP,2}, {"JMP", JMP,IND,5}, {"ADC", ADC,ABS,4}, {"ROR",mROR,ABS,6}, {"???", XXX,IMP,6},
	{"BVS", BVS,REL,2}, {"ADC", ADC,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,4}, {"ADC", ADC,ZPX,4}, {"ROR",mROR,ZPX,6}, {"???", XXX,IMP,6}, {"SEI", SEI,IMP,2}, {"ADC", ADC,ABY,4}, {"???", NOP,IMP,2}, {"???", XXX,IMP,7}, {"???", NOP,IMP,4}, {"ADC", ADC,ABX,4}, {"ROR",mROR,ABX,7}, {"???", XXX,IMP,7},
	{"???", NOP,IMP,2}, {"STA", STA,IZX,6}, {"???", NOP,IMP,2}, {"???", XXX,IMP,6}, {"STY", STY,ZP0,3}, {"STA", STA,ZP0,3}, {"STX", STX,ZP0,3}, {"???", XXX,IMP,3}, {"DEY", DEY,IMP,2}, {"???", NOP,IMP,2}, {"TXA", TXA,IMP,2}, {"???", XXX,IMP,2}, {"STY", STY,ABS,4}, {"STA", STA,ABS,4}, {"STX", STX,ABS,4}, {"???", XXX,IMP,4},
	{"BCC", BCC,REL,2}, {"STA", STA,IZY,6}, {"???", XXX,IMP,2}, {"???", XXX,IMP,6}, {"STY", STY,ZPX,4}, {"STA", STA,ZPX,4}, {"STX", STX,ZPY,4}, {"???", XXX,IMP,4}, {"TYA", TYA,IMP,2}, {"STA", STA,ABY,5}, {"TXS", TXS,IMP,2}, {"???", XXX,IMP,5}, {"???", NOP,IMP,5}, {"STA", STA,ABX,5}, {"???", XXX,IMP,5}, {"???", XXX,IMP,5},
	{"LDY", LDY,IMM,2}, {"LDA", LDA,IZX,6}, {"LDX", LDX,IMM,2}, {"???", XXX,IMP,6}, {"LDY", LDY,ZP0,3}, {"LDA", LDA,ZP0,3}, {"LDX", LDX,ZP0,3}, {"???", XXX,IMP,3}, {"TAY", TAY,IMP,2}, {"LDA", LDA,IMM,2}, {"TAX", TAX,IMP,2}, {"???", XXX,IMP,2}, {"LDY", LDY,ABS,4}, {"LDA", LDA,ABS,4}, {"LDX", LDX,ABS,4}, {"???", XXX,IMP,4},
	{"BCS", BCS,REL,2}, {"LDA", LDA,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,5}, {"LDY", LDY,ZPX,4}, {"LDA", LDA,ZPX,4}, {"LDX", LDX,ZPY,4}, {"???", XXX,IMP,4}, {"CLV", CLV,IMP,2}, {"LDA", LDA,ABY,4}, {"TSX", TSX,IMP,2}, {"???", XXX,IMP,4}, {"LDY", LDY,ABX,4}, {"LDA", LDA,ABX,4}, {"LDX", LDX,ABY,4}, {"???", XXX,IMP,4},
	{"CPY", CPY,IMM,2}, {"CMP", CMP,IZX,6}, {"???", NOP,IMP,2}, {"???", XXX,IMP,8}, {"CPY", CPY,ZP0,3}, {"CMP", CMP,ZP0,3}, {"DEC", DEC,ZP0,5}, {"???", XXX,IMP,5}, {"INY", INY,IMP,2}, {"CMP", CMP,IMM,2}, {"DEX", DEX,IMP,2}, {"???", XXX,IMP,2}, {"CPY", CPY,ABS,4}, {"CMP", CMP,ABS,4}, {"DEC", DEC,ABS,6}, {"???", XXX,IMP,6},
	{"BNE", BNE,REL,2}, {"CMP", CMP,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,4}, {"CMP", CMP,ZPX,4}, {"DEC", DEC,ZPX,6}, {"???", XXX,IMP,6}, {"CLD", CLD,IMP,2}, {"CMP", CMP,ABY,4}, {"NOP", NOP,IMP,2}, {"???", XXX,IMP,7}, {"???", NOP,IMP,4}, {"CMP", CMP,ABX,4}, {"DEC", DEC,ABX,7}, {"???", XXX,IMP,7},
	{"CPX", CPX,IMM,2}, {"SBC", SBC,IZX,6}, {"???", NOP,IMP,2}, {"???", XXX,IMP,8}, {"CPX", CPX,ZP0,3}, {"SBC", SBC,ZP0,3}, {"INC", INC,ZP0,5}, {"???", XXX,IMP,5}, {"INX", INX,IMP,2}, {"SBC", SBC,IMM,2}, {"NOP", NOP,IMP,2}, {"???", SBC,IMP,2}, {"CPX", CPX,ABS,4}, {"SBC", SBC,ABS,4}, {"INC", INC,ABS,6}, {"???", XXX,IMP,6},
	{"BEQ", BEQ,REL,2}, {"SBC", SBC,IZY,5}, {"???", XXX,IMP,2}, {"???", XXX,IMP,8}, {"???", NOP,IMP,4}, {"SBC", SBC,ZPX,4}, {"INC", INC,ZPX,6}, {"???", XXX,IMP,6}, {"SED", SED,IMP,2}, {"SBC", SBC,ABY,4}, {"NOP", NOP,IMP,2}, {"???", XXX,IMP,7}, {"???", NOP,IMP,4}, {"SBC", SBC,ABX,4}, {"INC", INC,ABX,7}, {"???", XXX,IMP,7},
};

// Sets size to the number of bytes the instruction takes
char* dissassemble(State6502* cpu, uint16_t addr, int* size)
{
	char* line = malloc(128);

	uint8_t opcode = cpu_bus_read(cpu->bus, addr);
	Instruction inst = opcodes[opcode];

	// Fetch the data for the instruction 
	uint8_t low = cpu_bus_read(cpu->bus, addr + 1);
	uint8_t high = cpu_bus_read(cpu->bus, addr + 2);

	// 12 Possible formats
	bool(*a)(State6502*) = inst.adressing_mode;
	if (a == IMM)
	{
		sprintf(line, "$%.4X: %s #$%.2X", addr, inst.code, low);
		if (size)
			*size = 2;
	}
	else if (a == ACC || a == IMP)
	{
		sprintf(line, "$%.4X: %s", addr, inst.code);
		if (size)
			*size = 1;
	}
	else if (a == ZP0)
	{
		sprintf(line, "$%.4X: %s *$%.2X", addr, inst.code, low);
		if (size)
			*size = 2;
	}
	else if (a == ZPX)
	{
		sprintf(line, "$%.4X: %s *$%.2X,X", addr, inst.code, low);
		if (size)
			* size = 2;
	}
	else if (a == ZPY)
	{
		sprintf(line, "$%.4X: %s *$%.2X,Y", addr, inst.code, low);
		if (size)
			* size = 2;
	}
	else if (a == REL)
	{
		sprintf(line, "$%.4X: %s $%.2X", addr, inst.code, low);
		if (size)
			* size = 2;
	}
	else if (a == IZX)
	{
		sprintf(line, "$%.4X: %s ($%.2X,X)", addr, inst.code, low);
		if (size)
			* size = 2;
	}
	else if (a == IZY)
	{
		sprintf(line, "$%.4X: %s ($%.2X),Y", addr, inst.code, low);
		if (size)
			* size = 2;
	}
	else if (a == ABS)
	{
		uint16_t operand = ((uint16_t)high << 8) | (uint16_t)low;
		sprintf(line, "$%.4X: %s $%.4X", addr, inst.code, operand);
		if (size)
			*size = 3;
	}
	else if (a == ABX)
	{
		uint16_t operand = ((uint16_t)high << 8) | (uint16_t)low;
		sprintf(line, "$%.4X: %s $%.4X,X", addr, inst.code, operand);
		if (size)
			* size = 3;
	}
	else if (a == ABY)
	{
		uint16_t operand = ((uint16_t)high << 8) | (uint16_t)low;
		sprintf(line, "$%.4X: %s $%.4X,Y", addr, inst.code, operand);
		if (size)
			* size = 3;
	}
	else if (a == IND)
	{
		uint16_t operand = ((uint16_t)high << 8) | (uint16_t)low;
		sprintf(line, "$%.4X: %s ($%.4X)", addr, inst.code, operand);
		if (size)
			* size = 3;
	}

	return line;
}