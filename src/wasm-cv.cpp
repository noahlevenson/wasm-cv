/*
* wasm-cv
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp -s TOTAL_MEMORY=512MB -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O1 -std=c++1z -o ../env/wasm-cv.js
*/

#include <iostream>
#include <algorithm>
#include <array>
#include <emscripten/emscripten.h>
#include "util.h"
#include "util.cpp"
#include "proc.h"
#include "proc.cpp"
#include "filt.h"
#include "filt.cpp"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* edgeFindStack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* rankStack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* colorMedian(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* morphStack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

EMSCRIPTEN_KEEPALIVE unsigned char* edgeFindStack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	unsigned char* buf1 = new unsigned char[size];
	unsigned char* buf2 = new unsigned char[size];
	buf1 = toGrayscale(inputBuf, buf1, w, h, size);
	buf2 = toBinary(buf1, buf2, w, h, size, 125);
	outputBuf = findEdges(buf2, outputBuf, w, size);
	delete [] buf1;
	delete [] buf2; 
	return outputBuf;
}

EMSCRIPTEN_KEEPALIVE unsigned char* medianStack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	unsigned char* buf1 = new unsigned char[size];
	buf1 = toGrayscale(inputBuf, buf1, w, h, size);
	outputBuf = tmf(buf1, outputBuf, w, size);
	delete [] buf1;
	return outputBuf;
}

EMSCRIPTEN_KEEPALIVE unsigned char* colorMedian(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	outputBuf = medianRGBA(inputBuf, outputBuf, w, size);
	return outputBuf;
}

EMSCRIPTEN_KEEPALIVE unsigned char* morphStack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	unsigned char* buf1 = new unsigned char[size];
	buf1 = toGrayscale(inputBuf, buf1, w, h, size);
	unsigned char* buf2 = new unsigned char[size];
	buf2 = toBinary(buf1, buf2, w, h, size, 150);
	//unsigned char* buf3 = new unsigned char[size];
	outputBuf = findEdges(buf2, outputBuf, w, size);
	//outputBuf = kDilate(buf3, outputBuf, w, size);
	delete [] buf1;
	delete [] buf2; 
	//delete [] buf3;
	return outputBuf;
}


#ifdef __cplusplus
}
#endif


