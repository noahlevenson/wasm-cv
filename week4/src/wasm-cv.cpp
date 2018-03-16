/*
* WASM-CV
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp util.cpp bina.cpp gray.cpp rgba.cpp -s TOTAL_MEMORY=1024MB -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../wasm-cv.js
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
EMSCRIPTEN_KEEPALIVE unsigned char* demoDilate5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* demoErode5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* demoOpen5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* demoClose5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE void update();

Wasmcv* project = new Wasmcv(640, 480);

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

EMSCRIPTEN_KEEPALIVE void update() {
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
	// Get a C++ pointer to the output buffer
	int outputBufInt = EM_ASM_INT(
		const outputBuf = Module._malloc(inputImgData.data.length);
		return outputBuf;
	);
	unsigned char* outputBuf = (unsigned char*)outputBufInt;
	// Do some C++ stuff
	unsigned char* buf1 = new unsigned char[project->size];
	buf1 = toGrayscale(inputBuf, buf1, project);
	outputBuf = threshold(buf1, outputBuf, project, 127);
	std::vector<int> segmentationMap = getConnectedComponents(outputBuf, project);
	std::vector<int> perimeterMap = getRegionPerimeter(segmentationMap, segmentationMap[615683], project);
	delete [] buf1;
	std::vector<int> boundingBox = getBoundingBox(perimeterMap, project);
	// Do some js stuff with C++ variables
	EM_ASM_({
		const outputImgData = new ImageData(webcamWidth, webcamHeight);
		for (var i = 0, len = outputImgData.data.length; i < len; i += 1) {
			outputImgData.data[i] = Module.HEAPU8[$0 + i];
		}
		outputCtx.putImageData(outputImgData, 0, 0);
		Module._free($1); // Passing a C++ variable to a javascript function that is calling a C++ function from within a C++ function
		Module._free($0); // Passing a C++ variable to a javascript function that is calling a C++ function from within a C++ function
		t2 = performance.now() - t1;
		outputCtx.font = '30px Arial';
		outputCtx.fillStyle = 'red';
		outputCtx.fillText(t2.toFixed(2) + ' ms', 10, 50);
		outputOverlayCtx.strokeStyle = "#005ce6";
		outputOverlayCtx.lineWidth = 4;
		outputOverlayCtx.beginPath();
		outputOverlayCtx.rect($2, $3, $4, $5); // Passing C++ variables to a javascript function
		loopId = window.requestAnimationFrame(Module._update);
	}, outputBuf, inputBuf, boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3]);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoDilate5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return kDilate5x5(inputBuf, outputBuf, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoErode5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return kErode5x5(inputBuf, outputBuf, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoOpen5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return open5x5(inputBuf, outputBuf, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoClose5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return close5x5(inputBuf, outputBuf, project, project->se._5x5iso);
}

#ifdef __cplusplus
}
#endif