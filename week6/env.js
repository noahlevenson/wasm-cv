// Globals for benchmarking
let t1, t2;

// Handle for the animationFrame loop
let loopId;

// Global for the imagedata object we read off the input canvas
let inputImgData;

// Identifiers for page elements
const outputCanvas = document.getElementById("output-canvas");
const outputOverlayCanvas = document.getElementById("output-overlay-canvas");
const inputCanvas = document.getElementById("input-canvas");
// const video = document.getElementById("video");

// Set input and output canvas dimensions
outputCanvas.width = 640;
outputCanvas.height = 480;

outputOverlayCanvas.width = 640;
outputOverlayCanvas.height = 480;

inputCanvas.width = 640;
inputCanvas.height = 480;

// Identifiers for canvas contexts
const outputCtx = outputCanvas.getContext("2d");
const outputOverlayCtx = outputOverlayCanvas.getContext("2d");
const inputCtx = inputCanvas.getContext("2d");

// Binding for the UI start button
function start() {
	loopId = window.requestAnimationFrame(Module._update);
	visualizeHaarD(128, 320, 120);
}

// Binding for the UI stop button
function stop() {
	window.cancelAnimationFrame(loopId);
}

function visualizeHaarA(s, sx, sy) {
	function* generateHaarA(s, sx, sy) {
		// Loop through every possible pixel position in a subwindow of dimensions s X s
		for (let i = 0; i < s; i += 1) {
			for (let j = 0; j < s; j += 1) {
				// Loop through every possible type A feature comprised of rectangles of size (w, h) such that the feature 
				// remains constrained by the subwindow dimensions
				for (let h = 1; i + h < s; h += 1) {
					for (let w = 1; w * 2 + j < s; w += 1) {
						const leftRectangle = [];
						leftRectangle[0] = sx + j;
						leftRectangle[1] = sy + i;
						leftRectangle[2] = w;
						leftRectangle[3] = h;
						const rightRectangle = [];
						rightRectangle[0] = sx + j + w;
						rightRectangle[1] = sy + i;
						rightRectangle[2] = w;
						rightRectangle[3] = h;
						const boundingBoxes = [];
						boundingBoxes.push(leftRectangle);
						boundingBoxes.push(rightRectangle);
						yield boundingBoxes;	
					}
				}
			}
		}
	}

	const v = generateHaarA(s, sx, sy);
	iterate(v);

	function iterate(v) {
		setTimeout(() => {
				const g = v.next();
				const boundingBoxes = g.value;
				const leftRectangle = boundingBoxes[0];
				const rightRectangle = boundingBoxes[1];
				outputOverlayCtx.clearRect(0, 0, 640, 480);
				outputOverlayCtx.strokeStyle = 'yellow';
				outputOverlayCtx.strokeRect(320, 120, 128, 128);
				outputOverlayCtx.fillStyle = 'white';
				outputOverlayCtx.fillRect(leftRectangle[0], leftRectangle[1], leftRectangle[2], leftRectangle[3]);
				outputOverlayCtx.fillStyle = 'black';
				outputOverlayCtx.fillRect(rightRectangle[0], leftRectangle[1], leftRectangle[2], leftRectangle[3]);
				if (!g.done) {
					iterate(v);
				}
		}, 1);
	}
}

function visualizeHaarD(s, sx, sy) {
	function* generateHaarD(s, sx, sy) {
		// Loop through every possible pixel position in a subwindow of dimensions s X s
		for (let i = 0; i <= s; i += 1) {
			for (let j = 0; j <= s; j += 1) {
				// Loop through every possible type D feature comprised of rectangles of size (w, h) such that the feature 
				// remains constrained by the subwindow dimensions
				for (let h = 1; i + h * 2 <= s; h += 1) {
					for (let w = 1; w * 2 + j <= s; w += 1) {
						const leftTopRectangle = [j + sx, i + sy, w, h];
						const rightBottomRectangle = [j + sx + w, i + sy + h, w, h];
						const rightTopRectangle = [j + sx + w, i + sy, w, h];
						const leftBottomRectangle = [j + sx, i + sy + h, w, h];
						const boundingBoxes = [];
						boundingBoxes.push(leftTopRectangle);
						boundingBoxes.push(rightBottomRectangle);
						boundingBoxes.push(leftBottomRectangle);
						boundingBoxes.push(rightTopRectangle);
						yield boundingBoxes;	
					}
				}
			}
		}
	}

	const v = generateHaarD(s, sx, sy);
	iterate(v);

	function iterate(v) {
		setTimeout(() => {
				const g = v.next();
				const boundingBoxes = g.value;
				const leftTopRectangle = boundingBoxes[0];
				const rightBottomRectangle = boundingBoxes[1];
				const leftBottomRectangle = boundingBoxes[2];
				const rightTopRectangle = boundingBoxes[3];
				outputOverlayCtx.clearRect(0, 0, 640, 480);
				outputOverlayCtx.strokeStyle = 'yellow';
				outputOverlayCtx.strokeRect(sx, sy, s, s);
				outputOverlayCtx.fillStyle = 'white';
				outputOverlayCtx.fillRect(leftTopRectangle[0], leftTopRectangle[1], leftTopRectangle[2], leftTopRectangle[3]);
				outputOverlayCtx.fillRect(rightBottomRectangle[0], rightBottomRectangle[1], rightBottomRectangle[2], rightBottomRectangle[3]);
				outputOverlayCtx.fillStyle = 'black';
				outputOverlayCtx.fillRect(leftBottomRectangle[0], leftBottomRectangle[1], leftBottomRectangle[2], leftBottomRectangle[3]);
				outputOverlayCtx.fillRect(rightTopRectangle[0], leftTopRectangle[1], leftTopRectangle[2], leftTopRectangle[3]);
				if (!g.done) {
					iterate(v);
				}
		}, 100);
	}
}


