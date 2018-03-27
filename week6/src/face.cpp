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
// Returns a 2D pixel-level representation
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project) {
	// 2D Data structure for our integral image
	std::vector<std::vector<int>> integral;
	integral.resize(project->w, std::vector<int>(project->h, 0));
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
		int yP = y - 1 < 0 ? 0 : sumTable[i - project->w * 4];  // northern neighbor
		int xP = x - 1 < 0 ? 0 : integral[x - 1][y];  // left hand neighbor
		sumTable[i] = yP + inputBuf[i];
		integral[x][y] = xP + sumTable[i];	
	}
	return integral;
}

// Compute the sum of pixels in an integral image within a rectangle of arbitrary size 
// 
// a +-----------------+ b
//   |                 |
//   |                 |
//   |                 |
//   |                 |
// d +-----------------+ c
EMSCRIPTEN_KEEPALIVE int getRectangleSum(std::vector<std::vector<int>>& integral, Wasmcv* project, int x, int y, int w, int h) {
	if (x != 0 && y != 0) {
		// CASE A: x != 0 && y != 0; sum is c + a - (b + d) 
		int a = integral[x][y];
		int b = integral[x + w][y];
		int c = integral[x + w][y + h];
		int d = integral[x][y + h];
		int sum = c + a - (b + d);
		return sum;
	} else if (x == 0 && y != 0) {
		// CASE B: x == 0; sum is c - b
		int b = integral[x + w][y];
		int c = integral[x + w][y + h];
		int sum = c - b;
		return sum;
	} else if (y == 0 && x != 0) {
		// CASE C: y == 0; sum is c - d
		int c = integral[x + w][y + h];
		int d = integral[x][y + h];
		int sum = c - d;
		return sum;
	} else {
		// CASE D: x == 0 && y == 0; sum is c
		int c = integral[x + w][y + h];
		return c;
	}	
}

// Compute Haar-like feature type A (as described in the Wang paper) over all possible scales and
// positions for a given subwindow pixel and location
EMSCRIPTEN_KEEPALIVE void computeHaarA(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type A feature comprised of rectangles of size (w, h) such that the feature 
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h < s; h += 1) {
				for (int w = 1; w * 2 + j < s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w, h);
					int blackSum = getRectangleSum(integral, project, j + sx + w, i + sy, w, h);
					int diff = blackSum - whiteSum;
				}
			}
		}
	}
}

// Compute Haar-like feature type B (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
EMSCRIPTEN_KEEPALIVE void computeHaarB(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type B feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h < s; h += 1) {
				for (int w = 1; w * 3 + j < s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w, h) + 
						getRectangleSum(integral, project, j + sx + w * 2, i + sy, w, h);
					int blackSum = getRectangleSum(integral, project, j + sx + w, i + sy, w, h);
					int diff = blackSum - whiteSum;
				}
			}
		}
	}
}

// Compute Haar-like feature type C (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
EMSCRIPTEN_KEEPALIVE void computeHaarC(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type C feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h * 2 < s; h += 1) {
				for (int w = 1; j + w < s; j += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w, h);
					int blackSum = getRectangleSum(integral, project, j + sx, i + sy + h, w, h);
					int diff = blackSum - whiteSum;
				}
			}
		}
	}
}

// Compute Haar-like feature type D (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
EMSCRIPTEN_KEEPALIVE void computeHaarD(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type D feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h * 3 < s; h += 1) {
				for (int w = 1; j + w < s; j += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w, h) + 
						getRectangleSum(integral, project, j + sx, i + sy + h * 2, w, h);
					int blackSum = getRectangleSum(integral, project, j + sx, i + sy + h, w, h);
					int diff = blackSum - whiteSum;
				}
			}
		}
	}
}

// Compute Haar-like feature type E (as described in the Wang paper) over all possible scales and 
// positions for a given subwindow size and location
EMSCRIPTEN_KEEPALIVE void computeHaarE(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type E feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h * 2 < s; h += 1) {
				for (int w = 1; j + w * 2 < s; j += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w, h) + 
						getRectangleSum(integral, project, j + sx + w, i + sy + h, w, h);
					int blackSum = getRectangleSum(integral, project, j + sx + w, i + sy, w, h) + 
						getRectangleSum(integral, project, j + sx, i + sy + h, w, h);
					int diff = blackSum - whiteSum;
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