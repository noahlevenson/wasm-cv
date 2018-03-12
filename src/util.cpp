#include <emscripten/emscripten.h>

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize a new project and get a pointer to its object
EMSCRIPTEN_KEEPALIVE Wasmcv* init(int w, int h) {
	Wasmcv* project = new Wasmcv(640, 480);
	return project;
}

// Check whether a given offset is within the image bounds of a
// Wasmcv project
EMSCRIPTEN_KEEPALIVE bool isInImageBounds(Wasmcv* project, int offset) {
	if (offset >= 0 && offset < project->size) {
		return true;
	} else {
		return false;
	}
}

#ifdef __cplusplus
}
#endif