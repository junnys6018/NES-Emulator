import Module from './emu.js';

const module = Module();

const PPU_CLOCK_FREQENCY = 5369318
const EVENT_NEW_FRAME = 1;
const EVENT_AUDIO_BUFFER_FULL = 2;
const EVENT_UNTIL_TICKS = 4;
const KEY_MAP = {
	"KeyX": 0,
	"KeyZ": 1,
	"KeyQ": 2,
	"Enter": 3,
	"ArrowUp": 4,
	"ArrowDown": 5,
	"ArrowLeft": 6,
	"ArrowRight": 7
};

class NES {
	constructor(rom, mount) {
		this._keys = 0;
		this._previousTimestamp = null;
		this._RAFRequestID = null;
		this._api = null;

		this._canvasContext = mount.getContext('2d');
		this._audioContext = new AudioContext();
		this._startTime = this._audioContext.currentTime;

		module.then(instance => {
			this._instance = instance;

			this._api = Object.freeze({
				initialize: instance.cwrap('initialize', null),
				createNes: instance.cwrap('create_nes', 'number', ['number', 'number', 'number', 'number']),
				freeNes: instance.cwrap('free_nes', null, ['number']),
				createBuffer: instance.cwrap('create_buffer', 'number', ['number']),
				freeBuffer: instance.cwrap('free_buffer', null, ['number']),
				emulateUntil: instance.cwrap('emulate_until', 'number', ['number', 'number']),
				getTotalCycles: instance.cwrap('get_total_cycles', 'number', ['number']),
				getFrameBuffer: instance.cwrap('get_framebuffer_wasm', 'number', ['number']),
				setKeys: instance.cwrap('set_keys', null, ['number', 'number']),
				getAudioBuffer: instance.cwrap('get_audio_buffer', 'number', []),
				flushAudioSamples: instance.cwrap('flush_audio_samples', 'number', ['number']),
			});

			this._api.initialize();

			const romsize = rom.byteLength;
			const buffer = this._api.createBuffer(romsize);
			const view = new Uint8Array(instance.HEAPU8.buffer, buffer, romsize);
			view.set(new Uint8Array(rom));

			this._nes = this._api.createNes(buffer, romsize, 0, 0);

			this._api.freeBuffer(buffer);
		})

		// Attach Event Listeners
		this._onKeyUp = this._onKeyUp.bind(this);
		window.addEventListener('keyup', this._onKeyUp);
		this._onKeyDown = this._onKeyDown.bind(this);
		window.addEventListener('keydown', this._onKeyDown);

		this._RAFCallback = this._RAFCallback.bind(this);
	}

	start() {
		this._RAFRequestID = requestAnimationFrame(this._RAFCallback);
	}

	shutdown() {
		window.removeEventListener('keyup', this._onKeyUp);
		window.removeEventListener('keydown', this._onKeyDown);

		this._api.freeNes(this._nes);

		if (this._RAFRequestID !== null) {
			cancelAnimationFrame(this._RAFRequestID);
		}
	}

	setKeyup(key) {
		this._keys &= ~(1 << key);
	}

	setKeyDown(key) {
		this._keys |= 1 << key;
	}

	_onKeyUp(e) {
		if (e.code in KEY_MAP) {
			const key = KEY_MAP[e.code];
			this.setKeyup(key);
		}
	}

	_onKeyDown(e) {
		if (this._audioContext.state === 'suspended') {
			this._audioContext.resume();
		}

		if (e.code in KEY_MAP) {
			const key = KEY_MAP[e.code];
			this.setKeyDown(key);
		}
	}

	_emulateUntil(cycle) {
		while (true) {
			const event = this._api.emulateUntil(this._nes, cycle);

			if (event & EVENT_NEW_FRAME) {
				const pixelPtr = this._api.getFrameBuffer(this._nes);
				const pixelView = new Uint8ClampedArray(this._instance.HEAPU8.buffer, pixelPtr, 240 * 256 * 4);
				const pixelData = new ImageData(pixelView, 256, 240);
				this._canvasContext.putImageData(pixelData, 0, 0);
			}

			if (event & EVENT_AUDIO_BUFFER_FULL) {
				const numSamples = this._api.flushAudioSamples(this._nes);

				if (numSamples !== 0 && this._audioContext.state === 'running') {
					const audioBufPtr = this._api.getAudioBuffer();
					const samples = new Float32Array(this._instance.HEAPF32.buffer, audioBufPtr, numSamples);
					const audioBuffer = this._audioContext.createBuffer(1, samples.length, 44100);
					audioBuffer.copyToChannel(samples, 0);

					const audioBufferSourceNode = this._audioContext.createBufferSource();
					audioBufferSourceNode.buffer = audioBuffer;
					audioBufferSourceNode.connect(this._audioContext.destination);
					if (this._startTime < this._audioContext.currentTime) {
						console.log(`Resetting audio, ${((this._audioContext.currentTime - this._startTime) * 1000).toFixed(2)}ms behind`);
						this._startTime = this._audioContext.currentTime + 0.1;
					}
					else if (this._startTime > this._audioContext.currentTime + 0.5) {
						console.log(`Audio Desync, ${((this._startTime - this._audioContext.currentTime) * 1000).toFixed(2)}ms ahead`);
					}
					audioBufferSourceNode.start(this._startTime);
					this._startTime += samples.length / 44100;
				}
			}

			if (event & EVENT_UNTIL_TICKS) {
				break;
			}
		}
	}

	_RAFCallback(timestamp) {
		this._RAFRequestID = requestAnimationFrame(this._RAFCallback);
		if (this._api !== null) {
			this._api.setKeys(this._nes, this._keys);

			const deltaTime = timestamp - (this._previousTimestamp ?? timestamp);
			const cycles = Math.min(deltaTime, 100) * PPU_CLOCK_FREQENCY / 1000;
			if (cycles > 0) {
				this._emulateUntil(this._api.getTotalCycles(this._nes) + BigInt(Math.round(cycles)));
			}
			this._previousTimestamp = timestamp;
		}
	}
}

const canvas = document.getElementById("nes");

const romname = 'blockoban.nes';
fetch(romname)
	.then(response => response.arrayBuffer())
	.then(buffer => {
		const nes = new NES(buffer, canvas);
		nes.start();
	});
