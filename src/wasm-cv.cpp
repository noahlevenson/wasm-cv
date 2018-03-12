/*
* WASM-CV
*
* source ./emsdk_env.sh --build=Release
*
* emcc wasm-cv.cpp util.cpp bina.cpp gray.cpp rgba.cpp -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../demo/week3/wasm-cv.js
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

int main() {
	std::cout << "Hello world! Love, C++ main()\n";
	return 0;
}

#ifdef __cplusplus
}
#endif