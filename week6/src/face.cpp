#include <emscripten/emscripten.h>
#include <vector>
#include <iostream>

#include "util.h"
#include "face.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create an integral image from a grayscale or binary image buffer
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

// Compute Haar-like feature type A (as described in the Wang paper) over all possible scales and
// positions in a given subwindow size -- the scale of feature type A is constrained by width
EMSCRIPTEN_KEEPALIVE void computeHaarA(unsigned char inputBuf[], Wasmcv* project, int s) {
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type A feature comprised of rectangles of size (w, h) such that the feature 
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h < s; h += 1) {
				for (int w = 1; w * 2 + j < s; w += 1) {

					
				}
			}
		}
	}
}

// Get geometry for Haar-like feature A at a given image subwindow position, relative position and scale
// The scale of feature type A is constrained by width
// Returns a std::vector of bounding boxes
// Useful for visualization
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> getGeomHaarA(int sx, int sy, int rx, int ry, int w, int h) {
	// TODO: Sanitize the inputs and disallow unconstrained features
	std::vector<int> leftRectangle(4);
	leftRectangle[0] = sx + rx;
	leftRectangle[1] = sy + ry;
	leftRectangle[2] = w;
	leftRectangle[3] = h;
	std::vector<int> rightRectangle(4);
	rightRectangle[0] = sx + rx + w;
	rightRectangle[1] = sy + ry;
	rightRectangle[2] = w;
	rightRectangle[3] = h;
	std::vector<std::vector<int>> boundingBoxes(2);
	boundingBoxes[0] = leftRectangle;
	boundingBoxes[1] = rightRectangle;
	return boundingBoxes;
}

#ifdef __cplusplus
}
#endif