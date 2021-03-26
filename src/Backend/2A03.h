#ifndef _2A03_H
#define _2A03_H
#include <stdint.h>
#include <stdbool.h>

#define AUDIO_SAMPLES (40)
#define SAMPLE_RATE  (44100)
#define SAMPLE_PERIOD (1.0f / SAMPLE_RATE)

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

typedef struct
{
	union
	{
		struct
		{
			uint8_t EV : 4; // Envelope period/volume
			uint8_t C : 1;	// Constant volume
			uint8_t L : 1;	// Loop envelope/disable length counter
			uint8_t D : 2;	// Duty cycle
		} bits;
		uint8_t reg;
	} vol;

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
	} sweep_register;

	uint8_t low;

	union
	{
		struct
		{
			uint8_t H : 3; // Timer high
			uint8_t L : 5; // Length counter load
		} bits;
		uint8_t reg;
	} high;

	divider timer;
	uint8_t sequencer;
	uint8_t sequence_sel;
	uint8_t length_counter;
	struct
	{
		divider div;
		uint8_t decay;
		bool start_flag;
	} envelope;

	struct
	{
		divider div;
		bool reload_flag;
		uint32_t target;
	} sweep;
} SquareChannel;

// Represents a snapshot of the audio buffer, used by the renderer to visualise each audio channel as a waveform
typedef struct
{
	float buffer[2048];
	int write_pos;

	// For DC cutoff filter
	float last_sample;
	float last_filter;
} AudioWindow;

void WindowInit(AudioWindow* win);
void WindowAddSample(AudioWindow* win, float sample);

// "2A03" APU implementation
typedef struct
{
	// Pulse 1 registers, write only, accessed from $4000-$4003
	SquareChannel SQ1;

	// Pulse 2 registers, write only, accessed from $4004-$4007
	SquareChannel SQ2;

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

	// DMC channel registers, write only, accessed from $4010-4013

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

	uint8_t DMC_LOAD_COUNTER;
	uint8_t DMC_ADDR;
	uint8_t DMC_LENGTH;

	uint8_t DMC_sample_buffer;	
	bool DMC_sample_buffer_empty;

	bool DMC_IRQ_flag;
	struct
	{
		uint16_t addr_counter;
		uint32_t bytes_remaining;
	} DMC_memory_reader;

	struct 
	{
		uint8_t shift_register;
		uint8_t bits_remaining;
		bool silence_flag;
	} DMC_output_unit;

	divider DMC_timer;

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
	bool frame_counter_IRQ_flag;

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

	// Stores audio samples at the native sample rate of 1.8MHz
	float audio_samples[AUDIO_SAMPLES];

	// Stores audio samples at a down sampled rate for our PC's audio driver to play
	float audio_buffer[4096];
	uint32_t audio_pos;
	float real_time;

	// Stores audio samples for each channel, to visualise the waveform
	AudioWindow SQ1_win;
	AudioWindow SQ2_win;
	AudioWindow TRI_win;
	AudioWindow NOISE_win;

	// Reference to cpu, since the apu can send interrupts
	struct State6502* cpu;
} State2A03;

void clock_2A03(State2A03* apu);
void reset_2A03(State2A03* apu);
void power_on_2A03(State2A03* apu);
void apu_write(State2A03* apu, uint16_t addr, uint8_t data);
uint8_t apu_read(State2A03* apu, uint16_t addr);

void apu_channel_set(State2A03* apu, int channel, bool en);

void AudioPrecompute();

#endif