#include "6502.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	char code[3]; // OP code of the instruction

	// Fetches data and loads it into cpu->operand for the instruction. Returns true if a page was crossed
	bool(*adressing_mode)(State6502* cpu);

	// Executes the instruction, changing the state of the cpu, some instructions care if a page boundary was crossed
	// to fetch its data. returns any additional cycles required to complete the instruction if any 
	uint32_t(*operation)(State6502* cpu, bool c);

	uint32_t cycles; // Number of cycles required for instruction to complete
} Instruction;

// 13 Adressing modes

// Accumilator addressing
bool ACC(State6502* cpu)
{
	return false;
}

// Immediate addressing
bool IMM(State6502* cpu)
{
	cpu->operand = bus_read(cpu->bus, cpu->PC++);
	return false;
}

// Zero page
bool ZP0(State6502* cpu)
{
	cpu->addr = bus_read(cpu->bus, cpu->PC++);
	cpu->operand = bus_read(cpu->bus, cpu->addr);

	return false;
}

// Zero page with X index
bool ZPX(State6502* cpu)
{
	uint16_t addr_low = bus_read(cpu->bus, cpu->PC++);
	cpu->addr = (addr_low + cpu->X) & 0x00FF;
	cpu->operand = bus_read(cpu->bus, cpu->addr);
	return false;
}

// Zero page with Y index
bool ZPY(State6502* cpu)
{
	uint16_t addr_low = bus_read(cpu->bus, cpu->PC++);
	cpu->addr = (addr_low + cpu->Y) & 0x00FF;
	cpu->operand = bus_read(cpu->bus, cpu->addr);
	return false;
}

// Absolulte
bool ABS(State6502* cpu)
{
	uint16_t addr_low = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = bus_read(cpu->bus, cpu->PC++);
	cpu->addr = (addr_high << 8) | addr_low;
	cpu->operand = bus_read(cpu->bus, cpu->addr);
	return false;
}

// Absolute with X index
bool ABX(State6502* cpu)
{
	uint16_t addr_low = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr = (addr_high << 8) | addr_low;
	cpu->addr = addr + cpu->X;
	cpu->operand = bus_read(cpu->bus, cpu->addr);

	return (addr & 0xFF00) != (cpu->addr & 0xFF00);
}

// Absolulte with Y index
bool ABY(State6502* cpu)
{
	uint16_t addr_low = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr = (addr_high << 8) | addr_low;
	cpu->addr = addr + cpu->Y;
	cpu->operand = bus_read(cpu->bus, cpu->addr);

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
	cpu->operand = bus_read(cpu->bus, cpu->PC++);
	return false;
}

// Indirect
bool IND(State6502* cpu)
{
	uint16_t addr_low = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_high = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr = (addr_high << 8) | addr_low;

	// Read high bits
	cpu->indirect_fetch = bus_read(cpu->bus, (addr + 1) & 0x00FF);
	cpu->indirect_fetch <<= 8;
	cpu->indirect_fetch |= bus_read(cpu->bus, addr);
	
	/* Hack: only the JMP instruction uses indirect addressing, however it could also be using absolute addressing
	 * If absolute addressing was used JMP would set the program counter to the value of cpu->operand, however
	 * if indirect addressing was used JMP would set the program counter to the value of cpu->absolute_indirect_fetch
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
	uint16_t zp_addr = bus_read(cpu->bus, cpu->PC++);
	zp_addr = (zp_addr + cpu->X) & 0x00FF;

	uint16_t addr_low = bus_read(cpu->bus, zp_addr);
	uint16_t addr_high = bus_read(cpu->bus, (zp_addr + 1) & 0x00FF);
	cpu->addr = (addr_high << 8) | addr_low;
	cpu->operand = bus_read(cpu->bus, cpu->addr);

	return false;
}

// Indirect with Y index
bool IZY(State6502* cpu)
{
	uint16_t zp_addr = bus_read(cpu->bus, cpu->PC++);
	uint16_t addr_low = bus_read(cpu->bus, zp_addr);
	uint16_t addr_high = bus_read(cpu->bus, (zp_addr + 1) & 0x00FF); // Read the next zero page address

	uint16_t addr = ((addr_high << 8) | addr_low);
	cpu->addr = addr + cpu->Y;
	cpu->operand = bus_read(cpu->bus, cpu->addr);
	return (addr & 0xFF00) != (cpu->addr & 0xFF00);
}

// 56 instructions

// Add with carry
uint32_t ADC(State6502* cpu, bool c)
{
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
	cpu->A = cpu->A & cpu->operand;

	// Set status flags
	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Arithmetic Shift Left TODO: FIXME
uint32_t ASL(State6502* cpu, bool c)
{
	uint16_t result = (uint16_t)cpu->A << 1;

	cpu->status.flags.C = (result & 0x0100) == 0x0100;
	cpu->status.flags.Z = (result & 0x00FF) == 0;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return 0;
}

// Branch if Carry Clear
uint32_t BCC(State6502* cpu, bool c)
{
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
	uint8_t result = cpu->A & cpu->operand;

	cpu->status.flags.Z = result == 0;
	cpu->status.flags.V = (cpu->operand & (1 << 6)) == (1 << 6);
	cpu->status.flags.N = (cpu->operand & (1 << 7)) == (1 << 7);

	return 0;
}

// Branch if Minus (negative flag set)
uint32_t BMI(State6502* cpu, bool c)
{
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
	// TODO
}

// Branch if Overflow Clear
uint32_t BVC(State6502* cpu, bool c)
{
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
	cpu->status.flags.C = (cpu->A >= cpu->operand);
	cpu->status.flags.Z = (cpu->A == cpu->operand);

	uint16_t result = (uint16_t)cpu->A - (uint16_t)cpu->operand;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Compare X Register
uint32_t CPX(State6502* cpu, bool c)
{
	cpu->status.flags.C = (cpu->X >= cpu->operand);
	cpu->status.flags.Z = (cpu->X == cpu->operand);

	uint16_t result = (uint16_t)cpu->X - (uint16_t)cpu->operand;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return 0;
}

// Compare Y Register
uint32_t CPY(State6502* cpu, bool c)
{
	cpu->status.flags.C = (cpu->Y >= cpu->operand);
	cpu->status.flags.Z = (cpu->Y == cpu->operand);

	uint16_t result = (uint16_t)cpu->Y - (uint16_t)cpu->operand;
	cpu->status.flags.N = (result & 0x0080) == 0x0080;

	return 0;
}

// Decrement Memory
uint32_t DEC(State6502* cpu, bool c)
{
	uint8_t result = cpu->operand - 1;
	bus_write(cpu->bus, cpu->addr, result);

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
	cpu->A = cpu->A ^ cpu->operand;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Increment Memory
uint32_t INC(State6502* cpu, bool c)
{
	uint8_t result = cpu->operand + 1;
	bus_write(cpu->bus, cpu->addr, result);

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
		cpu->PC = (uint16_t)cpu->operand;
	}

	return 0;
}

// Jump to Subroutine
uint32_t JSR(State6502* cpu, bool c)
{
	// TODO
}

// Load Accumulator
uint32_t LDA(State6502* cpu, bool c)
{
	cpu->A = cpu->operand;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Load X Register
uint32_t LDX(State6502* cpu, bool c)
{
	cpu->X = cpu->operand;

	cpu->status.flags.Z = cpu->X == 0;
	cpu->status.flags.N = (cpu->X & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Load Y Register
uint32_t LDY(State6502* cpu, bool c)
{
	cpu->Y = cpu->operand;

	cpu->status.flags.Z = cpu->Y == 0;
	cpu->status.flags.N = (cpu->Y & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Logical Shift Right
uint32_t LSR(State6502* cpu, bool c)
{
	// TODO
}


// No Operation
uint32_t NOP(State6502* cpu, bool c)
{
	return 0;
}

// Logical Inclusive OR
uint32_t ORA(State6502* cpu, bool c)
{
	cpu->A = cpu->A | cpu->operand;

	cpu->status.flags.Z = cpu->A == 0;
	cpu->status.flags.N = (cpu->A & 0x0080) == 0x0080;

	return c ? 1 : 0;
}

// Push Accumulator
uint32_t PHA(State6502* cpu, bool c)
{
	// TODO
}

// Push Processor Status
uint32_t PHP(State6502* cpu, bool c)
{
	// TODO
}

// Pull Accumulator
uint32_t PLA(State6502* cpu, bool c)
{
	// TODO
}

// Pull Processor Status
uint32_t PLP(State6502* cpu, bool c)
{
	// TODO
}

// Rotate Left
uint32_t ROL(State6502* cpu, bool c)
{
	// TODO
}

// Rotate Right
uint32_t ROR(State6502* cpu, bool c)
{
	// TODO
}

// Return from Interrupt
uint32_t RTI(State6502* cpu, bool c)
{
	// TODO
}

// Return from Subroutine
uint32_t RTS(State6502* cpu, bool c)
{
	// TODO
}

// Subtract with Carry
uint32_t SBC(State6502* cpu, bool c)
{
	uint16_t value = ((uint16_t)cpu->operand) ^ 0x00FF;

	uint16_t temp = (uint16_t)cpu->A + value + (uint16_t)cpu->status.flags.C;
	cpu->status.flags.C = (temp & 0xFF00) != 0;
	cpu->status.flags.Z = (temp & 0x00FF) == 0;
	cpu->status.flags.V = (temp ^ (uint16_t)cpu->A) & (temp ^ value) & 0x0080;
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
	bus_write(cpu->bus, cpu->addr, cpu->A);
	return 0;
}

// Store X Register
uint32_t STX(State6502* cpu, bool c)
{
	bus_write(cpu->bus, cpu->addr, cpu->X);
	return 0;
}

// Store Y Register
uint32_t STY(State6502* cpu, bool c)
{
	bus_write(cpu->bus, cpu->addr, cpu->Y);
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

	cpu->status.flags.Z = cpu->SP == 0;
	cpu->status.flags.N = (cpu->SP & 0x0080) == 0x0080;
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