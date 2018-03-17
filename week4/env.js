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

// Identifiers for canvas contexts
const outputCtx = outputCanvas.getContext("2d");
const outputOverlayCtx = outputOverlayCanvas.getContext("2d");
const inputCtx = inputCanvas.getContext("2d");

// Global identifiers for all the proc stack checkboxes on the page
let toGrayscale = document.getElementById("toGrayscale");
let threshold = document.getElementById("threshold");
let median = document.getElementById("median3x3");
let dilate = document.getElementById("kDilate5x5");
let erode = document.getElementById("kErode5x5");
let open = document.getElementById("open5x5");
let close = document.getElementById("close5x5");
let edges = document.getElementById("findEdges");
let corners = document.getElementById("findCorners");
let segments = document.getElementById("findSegments");
let segmentVisualizer = document.getElementById("visualizeSegments");
let centroids = document.getElementById("findCentroids");
let perimeter = document.getElementById("findPerimeter");
let bounding = document.getElementById("findBoundingBox");

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