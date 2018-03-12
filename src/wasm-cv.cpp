/*
* WASM-CV
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp -s TOTAL_MEMORY=1024MB -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../demo/week3/wasm-cv.js
*/

#include <iostream>
#include <algorithm>
#include <array>
#include <vector>
#include <emscripten/emscripten.h>
#include "util.h"
#include "util.cpp"
#include "rgba.h"
#include "rgba.cpp"
#include "gray.h"
#include "gray.cpp"
#include "bina.h"
#include "bina.cpp"
#include "demo.h"
#include "demo.cpp"

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