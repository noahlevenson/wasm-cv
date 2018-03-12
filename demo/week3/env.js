// Handle for the animationFrame loop
let loopId;

// Pointer to the wasmcv project object
let project;

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

// Get identifiers for all the proc stack checkboxes on the page
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

	outputOverlayCtx.clearRect(0, 0, 640, 480);

	stack(inputBuf, outputBuf, theStack);

	// Let's segment the image and get a pointer to a segmentation map
	const segMapPointer = Module.ccall("getConnectedComponents", "number", ["number", "number"], [outputBuf, project]);	

	if (perimeter.checked) {

		// Enable the ability to get bounding box
		bounding.disabled = false;

		// Get the region label for whatever region is directly under the reticle
		const targetLabel = Module.HEAP16[segMapPointer / Int16Array.BYTES_PER_ELEMENT + 615683]; 
		// 	 																			    ^ offset for A byte @ the center of the screen
		
		if (targetLabel > 0) {
			// Get the centroid for the region under the reticle
			const targetCentroid = Module.ccall("getRegionCentroid", "number", ["number", "number", "number"], [segMapPointer, targetLabel, project]);

			const centroidX = Module.HEAPU32[targetCentroid / Uint32Array.BYTES_PER_ELEMENT];
			const centroidY = Module.HEAPU32[targetCentroid / Uint32Array.BYTES_PER_ELEMENT + 1];

			// Get the area for the region under the reticle
			const targetArea = Module.ccall("getRegionArea", "number", ["number", "number", "number"], [segMapPointer, targetLabel, project]);

			// Get the locations of all perimieter pixels of the region label located under the reticle
			const perimeterPixelsPointer = Module.ccall("getRegionPerimeter", "number", ["number", "number", "number"], [segMapPointer, targetLabel, project]);

			// First get the length of the perimeter pixel offsets array, which is stored in its index 0
			let len = Module.HEAPU32[perimeterPixelsPointer / Uint32Array.BYTES_PER_ELEMENT];

			// Let's find the distances from centroid to each perimeter pixel and store them in an array
			let distances = [];
			for (let i = 1; i < len; i += 1) {
				let offsetToPerimeterPixel = Module.HEAPU32[perimeterPixelsPointer / Uint32Array.BYTES_PER_ELEMENT + i];

				// We need to convert our perimeter pixel imgdata offset to an x/y pair
				let pixelOffset = offsetToPerimeterPixel / 4;
				let x = pixelOffset % webcamWidth;
				let y = Math.floor(pixelOffset / webcamWidth);

		 		// Now you have x/y pairs for the centroid and for the current perimeter pixel...
		 		// how do we measure the distance between them? The formula for Euclidean distance, that's how!
		 		const d = Math.sqrt(Math.pow((x - centroidX), 2) + Math.pow((y - centroidY), 2));
		 		distances.push(d);
			}

			// Now we can calculate mean radial distance like so
			const meanRadDist = distances.reduce((a, c) => a + c, 0) / distances.length;

			// Now let's calculate standard deviation of radial distance
			const mapped = distances.map(x => Math.pow(x - meanRadDist, 2));
			const sdrd = mapped.reduce((a, c) => a + c, 0) / mapped.length;

			// Haralick's method for measurement of circularity is mean radial distance divided 
			// by standard deviation of radial distance
			const circularity = meanRadDist / sdrd;
		
			// Now I want to draw a visualization of the perimeter to the screen
			// Create a new imagedata object where we'll draw the perimeter
			const perimeterVisualized = new ImageData(640, 480);
		
			// Loop through the perimeter pixels offsets and draw them to the imagedata object
			for (let i = 1; i < len; i += 1) {
				let offsetToPerimeterPixel = Module.HEAPU32[perimeterPixelsPointer / Uint32Array.BYTES_PER_ELEMENT + i];
				perimeterVisualized.data[offsetToPerimeterPixel] = 102;
				perimeterVisualized.data[offsetToPerimeterPixel + 1] = 255;
				perimeterVisualized.data[offsetToPerimeterPixel + 2] = 51;
				perimeterVisualized.data[offsetToPerimeterPixel + 3] = 255;
			}
			// Draw the imagedata object to the canvas
			outputOverlayCtx.putImageData(perimeterVisualized, 0, 0);

			// Draw a green cross to mark our detected centroid
		 	outputOverlayCtx.strokeStyle = "#66ff33";
		 	outputOverlayCtx.lineWidth = 6;
			outputOverlayCtx.beginPath();
			outputOverlayCtx.moveTo(centroidX, centroidY - 11);
			outputOverlayCtx.lineTo(centroidX, centroidY + 11);
			outputOverlayCtx.stroke();
			outputOverlayCtx.beginPath();
			outputOverlayCtx.moveTo(centroidX - 11, centroidY);
			outputOverlayCtx.lineTo(centroidX + 11, centroidY);
			outputOverlayCtx.stroke();

			// Experimental: get bounding box
			if (bounding.checked) {
				const boundingBoxPointer = Module.ccall("getBoundingBox", "number", ["number", "number"], [perimeterPixelsPointer, project]);

				const boundingBoxX = Module.HEAPU32[boundingBoxPointer / Uint32Array.BYTES_PER_ELEMENT];
				const boundingBoxY = Module.HEAPU32[boundingBoxPointer / Uint32Array.BYTES_PER_ELEMENT + 1];
				const boundingBoxW = Module.HEAPU32[boundingBoxPointer / Uint32Array.BYTES_PER_ELEMENT + 2];
				const boundingBoxH = Module.HEAPU32[boundingBoxPointer / Uint32Array.BYTES_PER_ELEMENT + 3];

				// Experimental: Draw a bounding box!
				outputOverlayCtx.strokeStyle = "#005ce6";
				outputOverlayCtx.lineWidth = 4;
				outputOverlayCtx.beginPath();
				outputOverlayCtx.rect(boundingBoxX, boundingBoxY, boundingBoxW, boundingBoxH);
				outputOverlayCtx.stroke();
			}

			// Output some cool data to the canvas
			outputOverlayCtx.font = "30px Arial";
			outputOverlayCtx.fillStyle = "#66ff33";
			outputOverlayCtx.fillText("region # " + targetLabel, centroidX + 40, centroidY -120);
			outputOverlayCtx.fillText("area: " + targetArea, centroidX + 40, centroidY -80);
			outputOverlayCtx.fillText("circularity: " + circularity.toFixed(2), centroidX + 40, centroidY - 40);
		}

			// Draw a reticle at screen center 
			outputOverlayCtx.strokeStyle = "red";
		 	outputOverlayCtx.lineWidth = 2;
			outputOverlayCtx.beginPath();
			outputOverlayCtx.moveTo(320, 230);
			outputOverlayCtx.lineTo(320, 250);
			outputOverlayCtx.stroke();
			outputOverlayCtx.beginPath();
			outputOverlayCtx.moveTo(310, 240);
			outputOverlayCtx.lineTo(330, 240);
			outputOverlayCtx.stroke();
	} else {
		bounding.checked = false;
		bounding.disabled = true;
	}
																				  
	// Disable the centroids + segment visualizer checkboxes
	// because we cannot find centroids or visualize segments
	// without first segmenting the image
	centroids.disabled = true;
	segmentVisualizer.disabled = true;

	let segmentationMapPointer;

	if (!segments.checked) {
		centroids.checked = false;
	} else {
		// Enable the centroids checkbox because we now have a segmentation map
		centroids.disabled = false;
		segmentVisualizer.disabled = false;

		segmentationMapPointer = Module.ccall("getConnectedComponents", "number", ["number", "number"], [outputBuf, project]);	
	}

	if (segmentVisualizer.checked) {
		// Draw the segmentation map to the output canvas
		const visualized = new ImageData(640, 480);
		const r = Math.random();
		const g = Math.random();
		const b = Math.random();
		for (i = 3; i < 1228800; i += 4) {
			visualized.data[i - 3] = Module.HEAP16[segmentationMapPointer / Int16Array.BYTES_PER_ELEMENT + i] * r;
			visualized.data[i - 2] = Module.HEAP16[segmentationMapPointer / Int16Array.BYTES_PER_ELEMENT + i] * g;
			visualized.data[i - 1] = Module.HEAP16[segmentationMapPointer / Int16Array.BYTES_PER_ELEMENT + i] * b;
			visualized.data[i] = 255;
		}
		outputOverlayCtx.putImageData(visualized, 0, 0);
	}

	if (corners.checked) {
		const cornerOffsets = Module.ccall("findAllCorners", "number", ["number", "number"], [outputBuf, project]);
		for (let i = 1, len = Module.HEAPU32[cornerOffsets / Uint32Array.BYTES_PER_ELEMENT]; i < len; i += 1) {
			// Note that we have to do our own pointer arithmetic!
			let offset = Module.HEAPU32[cornerOffsets / Uint32Array.BYTES_PER_ELEMENT + i];
			let pixelOffset = offset / 4;
			let x = pixelOffset % webcamWidth;
			let y = Math.floor(pixelOffset / webcamWidth);
	    	// Draw nice pink cross marks to mark corners
	    	outputOverlayCtx.strokeStyle = "#ff33cc";
	    	outputOverlayCtx.lineWidth = 1;
	    	outputOverlayCtx.beginPath();
			outputOverlayCtx.moveTo(x, y - 11);
			outputOverlayCtx.lineTo(x, y + 11);
			outputOverlayCtx.stroke();
			outputOverlayCtx.beginPath();
			outputOverlayCtx.moveTo(x - 11, y);
			outputOverlayCtx.lineTo(x + 11, y);
			outputOverlayCtx.stroke();
		}
	}

	if (centroids.checked) {
		const allRegionCentroids = Module.ccall("getAllRegionCentroids", "number", ["number", "number", "number"], [segmentationMapPointer, 40, project]);
		for (n = 0; n < 1200; n += 1) {
			let offset = Module.HEAPU32[allRegionCentroids / Uint32Array.BYTES_PER_ELEMENT + n];
			if (offset !== 0) {
				let pixelOffset = offset / 4;
				let x = pixelOffset % webcamWidth;
			 	let y = Math.floor(pixelOffset / webcamWidth);
			 	// Draw a nice green cross to mark a centroid
			 	outputOverlayCtx.strokeStyle = "#66ff33";
			 	outputOverlayCtx.lineWidth = 6;
		    	outputOverlayCtx.beginPath();
				outputOverlayCtx.moveTo(x, y - 11);
				outputOverlayCtx.lineTo(x, y + 11);
				outputOverlayCtx.stroke();
				outputOverlayCtx.beginPath();
				outputOverlayCtx.moveTo(x - 11, y);
				outputOverlayCtx.lineTo(x + 11, y);
				outputOverlayCtx.stroke();
			}
		}
	}

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