/*
* WASM-CV
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp util.cpp bina.cpp gray.cpp rgba.cpp face.cpp -s TOTAL_MEMORY=512MB -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../wasm-cv.js
*
*/

#include <iostream>
#include <vector>
#include "util.h"
#include "bina.h"
#include "gray.h"
#include "rgba.h"
#include "face.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE void update();

Wasmcv* project = new Wasmcv(640, 480);
BufferPool* bufferPool = new BufferPool(640, 480);

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

EMSCRIPTEN_KEEPALIVE void update() {
	// Draw our Donny image to the offscreen canvas and convert it to an imagedata object
	EM_ASM(
		outputOverlayCtx.clearRect(0, 0, webcamWidth, webcamHeight);
		t1 = performance.now();
		inputCtx.drawImage(donny, 0, 0);
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
	
	// Convert Donny to grayscale and create an integral image intermediate representation
	unsigned char* grayscaled = toGrayscale(inputBuf, bufferPool, project);
	auto integral = makeIntegralImage(bufferPool->getCurrent(), project);

	computeHaarE(integral, project, 400, 0, 0);


	EM_ASM_({
		// Copy the input image to a new imagedata object and draw it to the output canvas
		const outputImgData = new ImageData(webcamWidth, webcamHeight);
		for (var i = 0, len = outputImgData.data.length; i < len; i += 1) {
			outputImgData.data[i] = Module.HEAPU8[$0 + i];
		}
		outputCtx.putImageData(outputImgData, 0, 0);
		// Free the input buffer pointer 
		Module._free($0); 
		// Draw a benchmark
		t2 = performance.now() - t1;
		outputCtx.font = '30px Arial';
		outputCtx.fillStyle = 'yellow';
		outputCtx.fillText(t2.toFixed(2) + ' ms', 10, 50);
	}, inputBuf);

	EM_ASM(
		// Iterate
		//window.requestAnimationFrame(Module._update);
	);
}

#ifdef __cplusplus
}
#endif