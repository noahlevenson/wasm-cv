/*
* wasm-cv
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../env/wasm-cv.js
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

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

EMSCRIPTEN_KEEPALIVE unsigned char* stack(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	
	unsigned char* buf1 = new unsigned char[size];

	buf1 = toGrayscale(inputBuf, outputBuf, w, h, size);

	// unsigned char* buf2 = new unsigned char[size];

	// buf2 = toBinary(buf1, buf2, w, h, size, 180);

	outputBuf = median(buf1, outputBuf, w, size);

	delete [] buf1;

	// delete [] buf2;

	return outputBuf;

}


#ifdef __cplusplus
}
#endif