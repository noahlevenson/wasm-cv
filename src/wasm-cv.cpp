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
#include "rgba.h"
#include "rgba.cpp"
#include "gray.h"
#include "gray.cpp"
#include "bina.h"
#include "bina.cpp"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* morphStack(unsigned char inputBuf[], unsigned char outputBuf[]);

// Declare the pointer to the project object as a global
// TODO: Maybe we can create an init function that returns a pointer to a project object to the javascript side
// and then the javascript can call all C++ functions with the function name and the pointer to the project object
Wasmcv* project;

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	project = new Wasmcv(640, 480);
	return 0;
}

EMSCRIPTEN_KEEPALIVE unsigned char* morphStack(unsigned char inputBuf[], unsigned char outputBuf[]) {
	unsigned char* buf1 = new unsigned char[project->size];
	buf1 = toGrayscale(inputBuf, buf1, project);
	unsigned char* buf2 = new unsigned char[project->size];
	buf2 = binarize(buf1, buf2, project, 150);
	unsigned char* buf3 = new unsigned char[project->size];
	buf3 = close5x5(buf2, buf3, project, project->se._5x5disc);
	outputBuf = findEdges(buf3, outputBuf, project);

	delete [] buf1;
	delete [] buf2; 
	delete [] buf3;
	return outputBuf;
}

#ifdef __cplusplus
}
#endif