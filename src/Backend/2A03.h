#ifndef _2A03_H
#define _2A03_H
#include <stdint.h>
#include <stdbool.h>

#define AUDIO_SAMPLES (40)

// Forward declaration to avoid circular dependency
struct State6502;
void IRQ_Set(struct State6502* cpu, int index);
void IRQ_Clear(struct State6502* cpu, int index);

enum
{
	CHANNEL_SQ1 = 1,
	CHANNEL_SQ2 = 1 << 1,
	CHANNEL_TRI = 1 << 2,
	CHANNEL_NOISE = 1 << 3,
	CHANNEL_DMC = 1 << 4,
};

// Common APU compnents
typedef struct
{
	uint32_t period; // actual period is one clock cycle more than given period
	uint32_t counter;
} divider;

bool clock_divider(divider* div);
void reload_divider(divider* div);

// "2A03" APU implementation
typedef struct
{
	// Pulse 1 registers, write only, accessed from $4000-$4003

	union
	{
		struct
		{
			uint8_t EV : 4; // Envelope period/volume
			uint8_t C  : 1; // Constant volume
			uint8_t L  : 1; // Loop envelope/disable length counter
			uint8_t D  : 2; // Duty cycle
		} bits;
		uint8_t reg;
	} SQ1_VOL; // $4000

	union
	{
		struct
		{
			uint8_t S : 3; // Shift count
			uint8_t N : 1; // Negative
			uint8_t P : 3; // Period
			uint8_t E : 1; // Enable
		} bits;
		uint8_t reg;
	} SQ1_SWEEP; // $4001

	uint8_t SQ1_LO; // $4002

	union
	{
		struct
		{
			uint8_t H : 3; // Timer high
			uint8_t L : 5; // Length counter load
		} bits;
		uint8_t reg;
	} SQ1_HI; // $4003

	divider SQ1_timer;
	uint8_t SQ1_sequencer;
	uint8_t SQ1_sequence_sel;
	uint8_t SQ1_length_counter;
	struct
	{
		divider div;
		uint8_t decay;
		bool start_flag;
	} SQ1_envelope;

	struct
	{
		divider div;
		bool reload_flag;
		uint32_t target;
	} SQ1_sweep;

	// Pulse 2 registers, write only, accessed from $4004-$4007

	union
	{
		struct
		{
			uint8_t EV : 4; // Envelope period/volume
			uint8_t C : 1; // Constant volume
			uint8_t L : 1; // Loop envelope/disable length counter
			uint8_t D : 2; // Duty cycle
		} bits;
		uint8_t reg;
	} SQ2_VOL; // $4004

	union
	{
		struct
		{
			uint8_t S : 3; // Shift count
			uint8_t N : 1; // Negative
			uint8_t P : 3; // Period
			uint8_t E : 1; // Enable
		} bits;
		uint8_t reg;
	} SQ2_SWEEP; // $4005

	uint8_t SQ2_LO; // $4006

	union
	{
		struct
		{
			uint8_t H : 3; // Timer high
			uint8_t L : 5; // Length counter load
		} bits;
		uint8_t reg;
	} SQ2_HI; // $4007

	divider SQ2_timer;
	uint8_t SQ2_sequencer;
	uint8_t SQ2_sequence_sel;
	uint8_t SQ2_length_counter;
	struct
	{
		divider div;
		uint8_t decay;
		bool start_flag;
	} SQ2_envelope;

	struct
	{
		divider div;
		bool reload_flag;
		uint32_t target;
	} SQ2_sweep;

	// Triangle channel registers, write only, accessed from $4008-$400B

	union
	{
		struct
		{
			uint8_t R : 7; // Linear counter reload
			uint8_t C : 1; // Control flag
		} bits;
		uint8_t reg;
	} TRI_LINEAR; // $4008

	uint8_t TRI_LOW; // Timer low 8 bits, $400A

	union
	{
		struct
		{
			uint8_t H : 3; // Timer high 3 bits
			uint8_t l : 5; // Length counter reload
		} bits;
		uint8_t reg;
	} TRI_HIGH; // Timer high bits and length counter $400B

	divider TRI_timer;
	uint8_t TRI_length_counter;
	uint8_t TRI_linear_counter;
	bool TRI_linear_reload_flag;
	int TRI_sequence_index;

	// Noise channel registers, write only, accessed from $400C-400F

	union
	{
		struct
		{
			uint8_t V : 4; // Volume
			uint8_t C : 1; // Constant volume flag
			uint8_t L : 1; // Envelope loop/ length counter halt
			uint8_t Unused : 2;
		} bits;
		uint8_t reg;
	} NOISE_VOL; // $400C

	union
	{
		struct
		{
			uint8_t P : 4; // Period
			uint8_t Unused : 3;
			uint8_t L : 1; // Loop flag
		} bits;
		uint8_t reg;
	} NOISE_PERIOD; // $400E

	union
	{
		struct
		{
			uint8_t Unused : 3;
			uint8_t L : 5; // Length counter load
		} bits;
		uint8_t reg;
	} NOISE_COUNT; // $400F

	divider NOISE_timer;
	uint8_t NOISE_length_counter;
	struct
	{
		divider div;
		uint8_t decay;
		bool start_flag;
	} NOISE_envelope;
	uint16_t NOISE_LFSR; // Linear feed back shift register (15 bits)

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
	uint32_t frame_count;
	bool IRQ_flag;

	// Counts the number of master clock cycles ie PPU cycles
	uint64_t total_cycles;

	// Counts the number of APU cycles (6 PPU cycles to 1 APU cycle)
	uint64_t apu_cycles;

	struct
	{
		// 4 bits
		uint8_t pulse1;
		uint8_t pulse2;
		uint8_t triangle;
		uint8_t noise;
	} channel_out;

	// Allows the emulator to enable/disable channels. For debugging
	int channel_enable;

	float audio_samples[AUDIO_SAMPLES];
	struct State6502* cpu;
} State2A03;

void clock_2A03(State2A03* apu);
void reset_2A03(State2A03* apu);
void power_on_2A03(State2A03* apu);
void apu_write(State2A03* apu, uint16_t addr, uint8_t data);
uint8_t apu_read(State2A03* apu, uint16_t addr);

float apu_filtered_sample(State2A03* apu);
void apu_channel_set(State2A03* apu, int channel, bool en);

void AudioInit();

#endif