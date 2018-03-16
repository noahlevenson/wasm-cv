// Globals for benchmarking
let t1, t2;

// Handle for the animationFrame loop
let loopId;

// Global for the imagedata object we read off the input canvas
let inputImgData;

// Identifiers for the webcam's native width/height...
const webcamWidth = 640;
const webcamHeight = 480;

// Identifiers for page elements
const outputCanvas = document.getElementById("output-canvas");
const outputOverlayCanvas = document.getElementById("output-overlay-canvas");
const inputCanvas = document.getElementById("input-canvas");
const video = document.getElementById("video");

// Set input and output canvas dimensions
outputCanvas.width = webcamWidth;
outputCanvas.height = webcamHeight;

outputOverlayCanvas.width = webcamWidth;
outputOverlayCanvas.height = webcamHeight;

inputCanvas.width = webcamWidth;
inputCanvas.height = webcamHeight;

// Grab canvas contexts
const outputCtx = outputCanvas.getContext("2d");
const outputOverlayCtx = outputOverlayCanvas.getContext("2d");
const inputCtx = inputCanvas.getContext("2d");

// Ask user's permission and then acess their webcam; getUserMedia() returns a promise that resolves 
// with a stream object which must be assigned as the srcObject of a video object
navigator.mediaDevices.getUserMedia({video : true}).then(function(stream) {
	video.srcObject = stream;
}).catch(function(err) {
    alert(err);
});

// Binding for the UI start button
function start() {
	loopId = window.requestAnimationFrame(Module._update);
}

// Binding for the UI stop button
function stop() {
	window.cancelAnimationFrame(loopId);
}