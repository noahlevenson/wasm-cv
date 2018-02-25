// Handle for the animationFrame loop
let loopId;

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

// Main loop
function update() {
	// Store the start time for benchmarking
	const t1 = performance.now();
	// Draw the current video frame to the input canvas
	inputCtx.drawImage(video, 0, 0);
	// Read the input canvas into an imagedata object
	const inputImgData = inputCtx.getImageData(0, 0, webcamWidth, webcamHeight);
	// Allocate space on the C++ heap for an array of bytes the length of our input imagedata object
	const inputBuf = Module._malloc(inputImgData.data.length * inputImgData.data.BYTES_PER_ELEMENT);
	// Now copy the input imagedata object to the buffer allocated above
	Module.HEAPU8.set(inputImgData.data, inputBuf);
	// Now allocate space on the C++ heap for an array of bytes the length of our output imagedata object
	// note that we can just use the dimensions and bytes per element from the input imgdata object
	const outputBuf = Module._malloc(inputImgData.data.length * inputImgData.data.BYTES_PER_ELEMENT);
	

	// Call the C++ function for the processing stack
	Module.ccall("morphStack", null, ["number", "number", "number", "number", "number"], 
		[inputBuf, outputBuf, inputImgData.width, inputImgData.height, inputImgData.data.length]);
	

	// Create an empty output imagedata object from the output buffer
	var outputImgData = new ImageData(inputImgData.width, inputImgData.height);
	// iterate through the heap and copy our output buffer to the output imagedata object
	for (i = 0, len = inputImgData.data.length; i < len; i += 1) {
		outputImgData.data[i] = Module.HEAPU8[outputBuf + i];

	}
	// Now just draw the output imagedata object to the output canvas!
	outputCtx.putImageData(outputImgData, 0, 0);
	// Free the space on the heap!
	Module._free(inputBuf);
	Module._free(outputBuf);


	//TODO: We shouldn't create a new output buffer on the heap every time... instead
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

// Binding for the UI start button
function start() {
	loopId = window.requestAnimationFrame(update);
}

// Binding for the UI stop button
function stop() {
	window.cancelAnimationFrame(loopId);
}