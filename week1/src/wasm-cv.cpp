/*
* WASM-CV
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp util.cpp bina.cpp gray.cpp rgba.cpp -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../demo/week4/wasm-cv.js
*
* -s TOTAL_MEMORY=1024MB
*/

#include <iostream>
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

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
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

EMSCRIPTEN_KEEPALIVE uint32_t* week4(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	unsigned char* buf1 = new unsigned char[project->size];
	buf1 = toGrayscale(inputBuf, buf1, project);
	unsigned char* buf2 = new unsigned char[project->size];
	buf2 = threshold(buf1, buf2, project, 127);
	int16_t* segmentationMap = getConnectedComponents(buf2, project);
	uint32_t* perimeterMap = getRegionPerimeter(segmentationMap, segmentationMap[615683], project);
	delete [] buf1;
	delete [] buf2;
	return getBoundingBox(perimeterMap, project);
}

#ifdef __cplusplus
}
#endif