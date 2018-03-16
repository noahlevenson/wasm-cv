// Handle for the animationFrame loop
let loopId;

// Pointer to the wasmcv project object
let project;

// Identifiers for the webcam's native width/height...
const webcamWidth = 640;
const webcamHeight = 480;

// Identifiers for page elements
const outputCanvas = document.getElementById("output-canvas");
const inputCanvas = document.getElementById("input-canvas");
const video = document.getElementById("video");

// Set input and output canvas dimensions
outputCanvas.width = webcamWidth;
outputCanvas.height = webcamHeight;

inputCanvas.width = webcamWidth;
inputCanvas.height = webcamHeight;

// Grab canvas contexts
const outputCtx = outputCanvas.getContext("2d");
const inputCtx = inputCanvas.getContext("2d");

// Ask user's permission and then acess their webcam; getUserMedia() returns a promise that resolves 
// with a stream object which must be assigned as the srcObject of a video object
navigator.mediaDevices.getUserMedia({video : true}).then(function(stream) {
	video.srcObject = stream;
}).catch(function(err) {
    alert(err);
});

// Get identifiers for all the proc stack checkboxes on the page
let toGrayscale = document.getElementById("toGrayscale");
let threshold = document.getElementById("threshold");
let median = document.getElementById("median3x3");
let dilate = document.getElementById("kDilate5x5");
let erode = document.getElementById("kErode5x5");
let open = document.getElementById("open5x5");
let close = document.getElementById("close5x5");
let edges = document.getElementById("findEdges");

// Main loop
function update() {
	// Store the start time for benchmarking
	const t1 = performance.now();
	// Draw the current video frame to the input canvas
	inputCtx.drawImage(video, 0, 0);
	// Read the input canvas into an imagedata object
	const inputImgData = inputCtx.getImageData(0, 0, webcamWidth, webcamHeight);
	// Allocate space on the C++ heap for an array of bytes the length of our input imagedata object
	const inputBuf = Module._malloc(inputImgData.data.length * inputImgData.data.BYTES_PER_ELEMENT); // TODO: Kill this needless multiplication (bytes per element = 1)
	// Now copy the input imagedata object to the buffer allocated above
	Module.HEAPU8.set(inputImgData.data, inputBuf);
	// Now allocate space on the C++ heap for an array of bytes the length of our output imagedata object
	// note that we can just use the dimensions and bytes per element from the input imgdata object
	const outputBuf = Module._malloc(inputImgData.data.length * inputImgData.data.BYTES_PER_ELEMENT); // TODO: Kill this needless multiplication (bytes per element = 1)
	let theStack = [];
	if (toGrayscale.checked) {
		theStack.push(["toGrayscale"]);
	}
	if (threshold.checked) {
		theStack.push(["threshold", 127]);
	}
	if (median.checked) {
		theStack.push(["median3x3"]);
	}
	if (dilate.checked) {
		theStack.push(["demoDilate5x5"]);
	}
	if (erode.checked) {
		theStack.push(["demoErode5x5"]);
	}
	if (open.checked) {
		theStack.push(["demoOpen5x5"]);
	}
	if (close.checked) {
		theStack.push(["demoClose5x5"]);
	}
	if (edges.checked) {
		theStack.push(["findEdges"])
	}
	stack(inputBuf, outputBuf, theStack);
	// Create an empty output imagedata object from the output buffer
	var outputImgData = new ImageData(inputImgData.width, inputImgData.height);
	// iterate through the heap and copy our output buffer to the output imagedata object
	for (let i = 0, len = inputImgData.data.length; i < len; i += 1) {
		outputImgData.data[i] = Module.HEAPU8[outputBuf + i];
	}
	// Now just draw the output imagedata object to the output canvas!
	outputCtx.putImageData(outputImgData, 0, 0);
	// Free the space on the heap!
	Module._free(inputBuf);
	Module._free(outputBuf);


	// TODO: We shouldn't create a new output buffer on the heap every time... instead
	// we should initialize one output buffer and just keep overwriting it... though I tried this and it was hella slow


	// Store the end time for benchmarking
	const t2 = performance.now() - t1;
	// Draw the benchmark to the output canvas
	outputCtx.font = "30px Arial";
	outputCtx.fillStyle = "red";
	outputCtx.fillText(t2.toFixed(2) + " ms", 10, 50);
	// Recursion
	loopId = window.requestAnimationFrame(update);
}

// Execute a stack of image processing functions
function stack(inputBuf, outputBuf, stackArray) {
	if (stackArray.length > 0) {
		for (let i = 0, len = stackArray.length; i < len; i += 1) {
			let argTypes = ["number", "number", "number"];
			let args = [inputBuf, outputBuf, project];
			if (stackArray[i].length > 1) {
				for (let k = 1; k < stackArray[i].length; k += 1) {
					argTypes.push("number");
					args.push(stackArray[i][k]);
				}	
			}
			let result = Module.ccall(stackArray[i][0], "number", argTypes, args);
			for (let j = 0; j < 1228800; j += 1) {
				Module.HEAPU8[inputBuf + j] = Module.HEAPU8[outputBuf + j];
			}
		}
	} else {
		for (let j = 0; j < 1228800; j += 1) {
			Module.HEAPU8[outputBuf + j] = Module.HEAPU8[inputBuf + j];
		}
	}
	
}

// Binding for the UI start button
function start() {
	// Init a new wasmcv project and store the pointer to its project object in our global identifier
	project = Module.ccall("init", "number", ["number", "number"], [640, 480]);
	loopId = window.requestAnimationFrame(update);
}

// Binding for the UI stop button
function stop() {
	window.cancelAnimationFrame(loopId);
}