#include <emscripten/emscripten.h>
#include <vector>
#include <iostream>

#include "util.h"
#include "face.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create an integral image from a binary image buffer
// The integral image at location (x, y) contains the sum of the pixels above and to the left of (x, y), inclusive
// Returns a representation as a 1:1 byte level map 
// TODO: Create a pixel-level intermediate representation? Should probably invent a global IR format
EMSCRIPTEN_KEEPALIVE std::vector<int> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project) {
	// Data structure for our integral image
	std::vector<int> integral(project->size);
	// Cache cumulative column values in a table
	std::vector<int> sumTable(project->size);
	// Loop through our input image 
	for (int i = 3; i < project->size; i += 4) {
		auto vec2 = offsetToVec2(i - 3, project);
		// Just for code comprehension
		int x = vec2[0];
		int y = vec2[1];
		// Value of our integral image at (x,y) should equal
		// the accumulated sum of the pixels in the column above it 
		// plus the accumulated sum of the pixels to the left
		int yP = y - 1 < 0 ? 0 : sumTable[i - project->w * 4];
		int xP = x - 1 < 0 ? 0 : integral[i - 4];
		sumTable[i] = yP + inputBuf[i];
		integral[i] = xP + sumTable[i];	
	}
	return integral;
}

#ifdef __cplusplus
}
#endif