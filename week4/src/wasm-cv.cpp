/*
* WASM-CV
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp util.cpp bina.cpp gray.cpp rgba.cpp -s TOTAL_MEMORY=512MB -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../wasm-cv.js
*
*/

#include <iostream>
#include <vector>
#include "util.h"
#include "bina.h"
#include "gray.h"
#include "rgba.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* demoDilate5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* demoErode5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* demoOpen5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* demoClose5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project);
EMSCRIPTEN_KEEPALIVE void update();

Wasmcv* project = new Wasmcv(640, 480);
BufferPool* bufferPool = new BufferPool(640, 480);

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoDilate5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	return kDilate5x5(inputBuf, pool, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoErode5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	return kErode5x5(inputBuf, pool, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoOpen5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	return open5x5(inputBuf, pool, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoClose5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	return close5x5(inputBuf, pool, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE void update() {
	// Get state of UI checkboxes
	int grayscaleChecked = EM_ASM_INT(return toGrayscale.checked);
	int thresholdChecked = EM_ASM_INT(return threshold.checked);
	int medianChecked = EM_ASM_INT(return median.checked);
	int dilateChecked = EM_ASM_INT(return dilate.checked);
	int erodeChecked = EM_ASM_INT(return erode.checked);
	int openChecked = EM_ASM_INT(return open.checked);
	int closeChecked = EM_ASM_INT(return close.checked);
	int edgesChecked = EM_ASM_INT(return edges.checked);
	int cornersChecked = EM_ASM_INT(return corners.checked);
	int segmentsChecked = EM_ASM_INT(return segments.checked);
	int segmentVisualizerChecked = EM_ASM_INT(return segmentVisualizer.checked);
	int centroidsChecked = EM_ASM_INT(return centroids.checked);
	int perimeterChecked = EM_ASM_INT(return perimeter.checked);
	int boundingChecked = EM_ASM_INT(return bounding.checked);
	// Do some js stuff
	EM_ASM(
		outputOverlayCtx.clearRect(0, 0, webcamWidth, webcamHeight);
		t1 = performance.now();
		inputCtx.drawImage(video, 0, 0);
		inputImgData = inputCtx.getImageData(0, 0, webcamWidth, webcamHeight);
	);
	// Copy the input imgdata object to the heap and get a C++ pointer to it
	// This is allocating space on the C++ heap using javascript from within a C++ function
	int inputBufInt = EM_ASM_INT(
		const inputBuf = Module._malloc(inputImgData.data.length);
		Module.HEAPU8.set(inputImgData.data, inputBuf);
		return inputBuf;
	);
	unsigned char* inputBuf = (unsigned char*)inputBufInt;
	// Do some C++ stuff
	bufferPool->copyToNew(inputBuf);
	if (grayscaleChecked) toGrayscale(bufferPool->getCurrent(), bufferPool, project);
	if (thresholdChecked) threshold(bufferPool->getCurrent(), bufferPool, project, 127);
	if (medianChecked) median3x3(bufferPool->getCurrent(), bufferPool, project);
	if (dilateChecked) demoDilate5x5(bufferPool->getCurrent(), bufferPool, project);
	if (erodeChecked) demoErode5x5(bufferPool->getCurrent(), bufferPool, project);
	if (openChecked) demoOpen5x5(bufferPool->getCurrent(), bufferPool, project);
	if (closeChecked) demoClose5x5(bufferPool->getCurrent(), bufferPool, project);
	if (edgesChecked) findEdges(bufferPool->getCurrent(), bufferPool, project);
	// Keep a pointer to the processed image for the left hand display
	unsigned char* processedImage = bufferPool->getCurrent();
	if (cornersChecked) {
		auto corners = findAllCorners(processedImage, project);
		// Do js stuff within a C++ for loop
		for (int i = 0; i < corners.size(); i += 1) {
			auto vec2 = offsetToVec2(corners[i], project);
			EM_ASM_({
				outputOverlayCtx.strokeStyle = '#ff33cc';
		    	outputOverlayCtx.lineWidth = 1;
		    	outputOverlayCtx.beginPath();
				outputOverlayCtx.moveTo($0, $1 - 11);
				outputOverlayCtx.lineTo($0, $1 + 11);
				outputOverlayCtx.stroke();
				outputOverlayCtx.beginPath();
				outputOverlayCtx.moveTo($0 - 11, $1);
				outputOverlayCtx.lineTo($0 + 11, $1);
				outputOverlayCtx.stroke();
			}, vec2[0], vec2[1]);
		}
	}
	if (segmentsChecked) {
		auto segmentationMap = getConnectedComponents(bufferPool->getCurrent(), project);
		EM_ASM(
			segmentVisualizer.disabled = false;
			centroids.disabled = false;
			perimeter.disabled = false;
		);
		if (segmentVisualizerChecked) {
			unsigned char* visualized = segMapToImage(segmentationMap, bufferPool, project);
			EM_ASM({
				const outputImgData = new ImageData(webcamWidth, webcamHeight);
				for (var i = 0, len = outputImgData.data.length; i < len; i += 1) {
					outputImgData.data[i] = Module.HEAPU8[$0 + i];
				}
				outputOverlayCtx.putImageData(outputImgData, 0, 0);
			}, visualized);
		} 
		if (centroidsChecked) {
			auto allCentroids = getAllRegionCentroids(segmentationMap, 40, project);
			for (int i = 0; i < allCentroids.size(); i += 1) {
				auto vec2 = offsetToVec2(allCentroids[i], project);
				EM_ASM({
					outputOverlayCtx.strokeStyle = '#66ff33';
				 	outputOverlayCtx.lineWidth = 6;
			    	outputOverlayCtx.beginPath();
					outputOverlayCtx.moveTo($0, $1 - 11);
					outputOverlayCtx.lineTo($0, $1 + 11);
					outputOverlayCtx.stroke();
					outputOverlayCtx.beginPath();
					outputOverlayCtx.moveTo($0 - 11, $1);
					outputOverlayCtx.lineTo($0 + 11, $1);
					outputOverlayCtx.stroke(); 
				}, vec2[0], vec2[1]);
			}
		}
		if (perimeterChecked) {
			EM_ASM(
				bounding.disabled = false;
			);
			auto perimeterMap = getRegionPerimeter(segmentationMap, segmentationMap[615683], project);
			unsigned char* perimeterMapImage = perimeterMapToImage(perimeterMap, bufferPool, project);
			EM_ASM({
				const outputImgData = new ImageData(webcamWidth, webcamHeight);
				for (var i = 0, len = outputImgData.data.length; i < len; i += 1) {
					outputImgData.data[i] = Module.HEAPU8[$0 + i];
				}
				outputOverlayCtx.putImageData(outputImgData, 0, 0);
			}, perimeterMapImage);
			EM_ASM({
				outputOverlayCtx.strokeStyle = '#ff0000';
			 	outputOverlayCtx.lineWidth = 2;
		    	outputOverlayCtx.beginPath();
				outputOverlayCtx.moveTo($0, $1 - 11);
				outputOverlayCtx.lineTo($0, $1 + 11);
				outputOverlayCtx.stroke();
				outputOverlayCtx.beginPath();
				outputOverlayCtx.moveTo($0 - 11, $1);
				outputOverlayCtx.lineTo($0 + 11, $1);
				outputOverlayCtx.stroke(); 
			}, project->centerX, project->centerY);
			if (boundingChecked) {
				auto boundingBox = getBoundingBox(perimeterMap, project);
				EM_ASM({
					outputOverlayCtx.strokeStyle = '#005ce6';
					outputOverlayCtx.lineWidth = 4;
					outputOverlayCtx.beginPath();
					outputOverlayCtx.rect($0, $1, $2, $3); // Passing C++ variables to a javascript function
					outputOverlayCtx.stroke();
				}, boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3]);
			}
		} else {
			EM_ASM(
				bounding.checked = false;
				bounding.disabled = true;
			);
		}
	} else {
		EM_ASM(
			segmentVisualizer.checked = false;
			segmentVisualizer.disabled = true;
			centroids.checked = false;
			centroids.disabled = true;
			perimeter.checked = false;
			perimeter.disabled = true;
			bounding.checked = false;
			bounding.disabled = true;
		);
	}
	// Do some js stuff with C++ variables
	EM_ASM_({
		const outputImgData = new ImageData(webcamWidth, webcamHeight);
		for (var i = 0, len = outputImgData.data.length; i < len; i += 1) {
			outputImgData.data[i] = Module.HEAPU8[$0 + i];
		}
		outputCtx.putImageData(outputImgData, 0, 0);
		Module._free($1); // Passing a C++ variable to a javascript function that is calling a C++ function from within a C++ function
		t2 = performance.now() - t1;
		outputCtx.font = '30px Arial';
		outputCtx.fillStyle = 'red';
		outputCtx.fillText(t2.toFixed(2) + ' ms', 10, 50);
		loopId = window.requestAnimationFrame(Module._update);
	}, processedImage, inputBuf);
}

#ifdef __cplusplus
}
#endif