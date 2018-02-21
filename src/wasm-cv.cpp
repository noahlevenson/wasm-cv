/*
* wasm-cv
*
* emcc wasm-cv.cpp -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', '_free']" -s WASM=1 -O3 -std=c++1z -o ../env/wasm-cv.js
*/

#include <emscripten/emscripten.h>
#include <iostream>
#include "util.h"
#include "util.cpp"
#include "processing.h"
#include "processing.cpp"

#ifdef __cplusplus
extern "C" {
#endif

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

#ifdef __cplusplus
}
#endif