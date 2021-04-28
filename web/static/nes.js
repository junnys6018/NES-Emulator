importScripts("emu.js", "message.js");

let api;
let ready = false;

let keys = new Uint8Array(8);
keys.fill(0);

function Frame() {
	let t1 = performance.now();

	api.SetKeys(keys);
	api.EmulateFrame();
	
	let pixelPtr = api.GetFrameBuffer();
	let pixelView = new Uint8ClampedArray(Module.HEAPU8.buffer, pixelPtr, 240 * 256 * 4);
	let pixelData = new ImageData(pixelView, 256, 240);

	let numSamples = api.FlushAudioSamples();
	let audioBufPtr = api.GetAudioBuffer();
	if (numSamples !== 0) {
		let sampleView = new Float32Array(Module.HEAPF32.buffer, audioBufPtr, numSamples); // view of wasm memory
		postMessage({ type: MESSAGE_TYPE.SEND_AUDIO, audio: new Float32Array(sampleView) }); // we pass a copy of the view
	}

	let t2 = performance.now();
	totalTime += t2 - t1;
	numFrames++;
	postMessage({type: MESSAGE_TYPE.SEND_PIXELS, pixels: pixelData});
}

onmessage = function (e) {
	if (ready) {
		switch (e.data.type) {
		case MESSAGE_TYPE.KEY_UP:
			keys[e.data.key] = 0;
			break;
		case MESSAGE_TYPE.KEY_DOWN:
			keys[e.data.key] = 1;
			break;
		case MESSAGE_TYPE.LOAD_ROM:
			api.LoadRom(e.data.rom);
				break;
		case MESSAGE_TYPE.PRINT_TIME:
			printTime();
			break;
		case MESSAGE_TYPE.RESET_TIME:
			resetTime();
			break;
		}
	}
};

let totalTime = 0;
let numFrames = 0;

function printTime() {
	console.log(`${numFrames} frames took ${totalTime}ms (avg: ${totalTime / numFrames})`);
}

function resetTime() {
	totalTime = 0;
	numFrames = 0;
}

Module.onRuntimeInitialized = async _ => {
	api = Object.freeze({
		LoadRom: Module.cwrap('LoadRom', null, ['string']),
		EmulateFrame: Module.cwrap('EmulateFrame', null),
		GetFrameBuffer: Module.cwrap('GetFrameBuffer', 'number'),
		SetKeys: Module.cwrap('SetKeys', null, ['array']),
		GetAudioBuffer: Module.cwrap('GetAudioBuffer', 'number'),
		FlushAudioSamples: Module.cwrap('FlushAudioSamples', 'number'),
	});
	api.LoadRom("package/blockoban.nes");
	console.log("loaded emu");

	ready = true;

	setInterval(Frame, 16);
};

function Benchmark() {
	let frames = 20000;
	let t1 = performance.now();
	for (let i = 0; i < frames; i++) {
		api.EmulateFrame();
	}
	let t2 = performance.now();
	console.log(`emulated ${frames} frames in ${t2 - t1}ms (avg: ${(t2 - t1) / frames}ms)`);
}