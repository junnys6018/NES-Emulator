const PPU_CLOCK_FREQENCY = 5369318
const EMULATION_EVENT = Object.freeze({
	EVENT_NEW_FRAME: 1,
	EVENT_AUDIO_BUFFER_FULL: 2,
	EVENT_UNTIL_TICKS: 4
});
const KEY_MAP = Object.freeze({
	"x": 0, "X": 0,
	"z": 1, "Z": 1,
	"q": 2, "Q": 2,
	"Enter": 3,
	"ArrowUp": 4,
	"ArrowDown": 5,
	"ArrowLeft": 6,
	"ArrowRight": 7
});

let api;
let keys = new Uint8Array(8);
keys.fill(0);
const canvas = document.getElementById("nes");
const canvasContext = canvas.getContext("2d");
const audioContext = new AudioContext();
let startTime = audioContext.currentTime;
audioContext.suspend();

// Input 
window.addEventListener("keyup", (e) => {
	if (e.key in KEY_MAP) {
		keys[KEY_MAP[e.key]] = 0;
	}
});

window.addEventListener("keydown", (e) => {
	if (e.key in KEY_MAP) {
		keys[KEY_MAP[e.key]] = 1;
	}
});

document.getElementById("rom-select").onchange = (e) => {
	api.LoadRom(e.target.value);
}

document.getElementById("audio").onchange = (e) => {
	if (e.target.checked) {
		audioContext.suspend();
	}
	else {
		audioContext.resume();
		startTime = audioContext.currentTime;
	}
}

let lastTimestamp;
function RAFCallback(timestamp) {
	requestAnimationFrame(RAFCallback);
	api.SetKeys(keys);

	const deltaTime = (timestamp - (lastTimestamp || timestamp));
	const cycles = Math.min(deltaTime, 100) * PPU_CLOCK_FREQENCY / 1000;
	if (cycles > 0) {
		EmulateUntil(api.GetTotalPPUCycles() + cycles);
	}
	lastTimestamp = timestamp;
}

function EmulateUntil(cycle) {
	while (true) {
		const event = api.EmulateUntil(cycle);
		if (event & EMULATION_EVENT.EVENT_NEW_FRAME) {
			const pixelPtr = api.GetFrameBuffer();
			const pixelView = new Uint8ClampedArray(Module.HEAPU8.buffer, pixelPtr, 240 * 256 * 4);
			const pixelData = new ImageData(pixelView, 256, 240);
			canvasContext.putImageData(pixelData, 0, 0);
		}
		if (event & EMULATION_EVENT.EVENT_AUDIO_BUFFER_FULL) {
			const numSamples = api.FlushAudioSamples();
			if (numSamples !== 0 && audioContext.state === 'running') {
				const audioBufPtr = api.GetAudioBuffer();
				const samples = new Float32Array(Module.HEAPF32.buffer, audioBufPtr, numSamples);
				const audioBuffer = audioContext.createBuffer(1, samples.length, 44100);
				audioBuffer.copyToChannel(samples, 0);
				
				const audioBufferSourceNode = audioContext.createBufferSource();
				audioBufferSourceNode.buffer = audioBuffer;
				audioBufferSourceNode.connect(audioContext.destination);
				if (startTime < audioContext.currentTime) {
					console.log(`Resetting audio, ${((audioContext.currentTime - startTime) * 1000).toFixed(2)}ms behind`);
					startTime = audioContext.currentTime + 0.1;
				}
				else if (startTime > audioContext.currentTime + 0.5) {
					console.log(`Audio Desync, ${((startTime - audioContext.currentTime) * 1000).toFixed(2)}ms ahead`);
				}
				audioBufferSourceNode.start(startTime);
				startTime += samples.length / 44100;
			}
		}
		if (event & EMULATION_EVENT.EVENT_UNTIL_TICKS) {
			break;
		}
	}
}

Module.onRuntimeInitialized = async _ => {
	api = Object.freeze({
		LoadRom: Module.cwrap('LoadRom', null, ['string']),
		EmulateUntil: Module.cwrap('EmulateUntil', 'number', ['number']),
		GetFrameBuffer: Module.cwrap('GetFrameBuffer', 'number'),
		SetKeys: Module.cwrap('SetKeys', null, ['array']),
		GetAudioBuffer: Module.cwrap('GetAudioBuffer', 'number'),
		FlushAudioSamples: Module.cwrap('FlushAudioSamples', 'number'),
		GetTotalPPUCycles: Module.cwrap('GetTotalPPUCycles', 'number'),
	});
	api.LoadRom("package/blockoban.nes");
	console.log("loaded emu");

	requestAnimationFrame(RAFCallback);
};
