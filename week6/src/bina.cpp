#include <emscripten/emscripten.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

#include "util.h"
#include "bina.h"

#ifdef __cplusplus
extern "C" {
#endif

// Get sum of 3x3 neighbor pixel values for a binary thresholded image
// Note that this is NEIGHBORS ONLY, does not include p0 value
int sumNeighbors(unsigned char inputBuf[], int p0, int w) {
	return inputBuf[p0 - 4 - w * 4] + inputBuf[p0 - w * 4] + inputBuf[p0 + 4 - w * 4] + inputBuf[p0 - 4] + inputBuf[p0 + 4] + 
		inputBuf[p0 - 4 + w * 4] + inputBuf[p0 + w * 4] + inputBuf[p0 + 4 + w * 4];
}

// Convolve an binary thresholded image with a 3x3 kernel
unsigned char* conv3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int k[], int norm) {
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = ((inputBuf[i - 4 - project->w * 4] * k[0]) + (inputBuf[i - project->w * 4] * k[1]) + (inputBuf[i + 4 - project->w * 4] * k[2]) + 
			(inputBuf[i - 4] * k[3]) + (inputBuf[i] * k[4]) + (inputBuf[i + 4] * k[5]) + (inputBuf[i- 4 + project->w * 4] * k[6]) + 
				(inputBuf[i + project->w * 4] * k[7]) + (inputBuf[i + 4 + project->w * 4] * k[8])) / norm;
	}
	return outputBuf;
}

// Simple dilate
EMSCRIPTEN_KEEPALIVE unsigned char* dilate(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	for (int i = 3; i < project->size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, project->w);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = sigma > 0 ? 255 : inputBuf[i];
	}
	return outputBuf;
}

// Simple erode
EMSCRIPTEN_KEEPALIVE unsigned char* erode(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	for (int i = 3; i < project->size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, project->w);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = sigma < 2040 ? 0 : inputBuf[i];
	}
	return outputBuf;
}

// Binary dilation using a 3x3 structuring kernel
// If dilation finds a black input pixel, then it ORs the entire structuring kernel, 
// centered over the input pixel, to the output image
// The effect is that black pixel regions "grow" in approximately the shape of the structuring kernel
// TODO: Optimize this - there's a faster implementation
unsigned char* kDilate3x3(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* outputBuf = pool->getNew();
	std::array<int, 9> o = project->offsets._3x3;
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		if (inputBuf[i] == 255) {
			for (int j = 0; j < 9; j += 1) { 
				if (i + o[j] >= 0 && i + o[j] < project->size) {
					outputBuf[i + o[j]] = k.kernel[j] == 255 || inputBuf[i + o[j]] == 255 ? 255 : 0;	
				}
			}
		} else {
			outputBuf[i] = 0;
		}
	}
	return outputBuf;
}

// Binary dilation using a 5x5 structuring kernel
// If dilation finds a black input pixel, then it ORs the entire structuring kernel, 
// centered over the input pixel, to the output image
// The effect is that black pixel regions "grow" in approximately the shape of the structuring kernel
// TODO: Optimize this - there's a faster implementation
unsigned char* kDilate5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* outputBuf = pool->getNew();
	std::array<int, 25> o = project->offsets._5x5;
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		if (inputBuf[i] == 255) {
			for (int j = 0; j < 25; j += 1) { 
				if (i + o[j] >= 0 && i + o[j] < project->size) {
					outputBuf[i + o[j]] = k.kernel[j] == 255 || inputBuf[i + o[j]] == 255 ? 255 : 0;	
				}
			}
		} else {
			outputBuf[i] = 0;
		}
	}
	return outputBuf;
}

// Binary erosion using a 3x3 structuring kernel
// We center the structuring element over each input pixel and check whether every black pixel
// in the structuring element corresponds to a black pixel in the image beneath it. If yes, 
// then we OR the center value of our structuring element with our centered input pixel
// The effect is that white pixel regions "grow" in approximately the shape of the structuring kernel
// TODO: Optimize this - there's a faster implementation
unsigned char* kErode3x3(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* outputBuf = pool->getNew();
	std::array<int, 9> o = project->offsets._3x3;
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		int hits = 0;
		for (int j = 0; j < 9; j += 1) {
			if (k.kernel[j] == 255 && inputBuf[i + o[j]] == 255) {
				hits += 1;
			}
		}
		outputBuf[i] = hits == k.positives ? inputBuf[i] : 0;	
	}
	return outputBuf;
}

// Binary erosion using a 5x5 structuring kernel
// TODO: Optimize this - there's a faster implementation
unsigned char* kErode5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* outputBuf = pool->getNew();
	std::array<int, 25> o = project->offsets._5x5;
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		int hits = 0;
		for (int j = 0; j < 25; j += 1) {
			if (k.kernel[j] == 255 && inputBuf[i + o[j]] == 255) {
				hits += 1;
			}
		}
		outputBuf[i] = hits == k.positives ? inputBuf[i] : 0;
	}
	return outputBuf;
}

// Find edges (simple)
// Finds neighborhoods that are entirely black pixels and replaces their p0 with a white value
// the result is that deep black "fill" areas are turned white, while the edges of those fill areas are preserved as black
EMSCRIPTEN_KEEPALIVE unsigned char* findEdges(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	unsigned char* outputBuf = pool->getNew();
	for (int i = 3; i < project->size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, project->w);
		outputBuf[i] = sigma == 2040 ? 0 : inputBuf[i];
	}
	return outputBuf;
}

// Remove salt and pepper noise (simple)
EMSCRIPTEN_KEEPALIVE unsigned char* deSaltPepper(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	for (int i = 3; i < project->size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, project->w);
		if (sigma == 0) {
			outputBuf[i] = 0;
		} else if (sigma == 2040) {
			outputBuf[i] = 255;
		} else {
			outputBuf[i] = inputBuf[i];
		}
	}
	return outputBuf;
}

// Noise suppression by neighborhood averaging (box blur)
EMSCRIPTEN_KEEPALIVE unsigned char* boxBlur(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	int k[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	return conv3(inputBuf, outputBuf, project, k, 9);
}

// Improved noise suppression by neighborhood averaging (Gaussian approximation)
EMSCRIPTEN_KEEPALIVE unsigned char* gaussianApprox(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	int k[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
	return conv3(inputBuf, outputBuf, project, k, 16);
}

// Binary morphological closing using a 3x3 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* close3x3(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement3x3 k) {
	kDilate3x3(inputBuf, pool, project, k);
	kErode3x3(pool->getCurrent(), pool, project, k);
	return pool->getCurrent();
}

// Binary morphological closing using a 5x5 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* close5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement5x5 k) {
	kDilate5x5(inputBuf, pool, project, k);
	kErode5x5(pool->getCurrent(), pool, project, k);
	return pool->getCurrent();
}

// Binary morphological opening using a 3x3 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* open3x3(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement3x3 k) {
	kErode3x3(inputBuf, pool, project, k);
	kDilate3x3(pool->getCurrent(), pool, project, k);
	return pool->getCurrent();
}

// Binary morphological opening using a 3x3 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* open5x5(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement5x5 k) {
	kErode5x5(inputBuf, pool, project, k);
	kDilate5x5(pool->getCurrent(), pool, project, k);
	return pool->getCurrent();
}

// Subtract an image from an image
EMSCRIPTEN_KEEPALIVE unsigned char* sub(unsigned char inputBufA[], unsigned char inputBufB[], BufferPool* pool, Wasmcv* project) {
	unsigned char* outputBuf = pool->getNew();
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = inputBufA[i] - inputBufB[i];	
	}
	return outputBuf;
}

// White top hat (residue find) operator using a 3x3 structuring element
// = the difference between an input image and its opening 
EMSCRIPTEN_KEEPALIVE unsigned char* topHat3x3White(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* openedImage = open3x3(inputBuf, pool, project, k);
	unsigned char* outputBuf = sub(inputBuf, openedImage, pool, project);
	return outputBuf;
}

// // White top hat (residue find) operator using a 5x5 structuring element
// // = the difference between an input image and its opening 
EMSCRIPTEN_KEEPALIVE unsigned char* topHat5x5White(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* openedImage = open5x5(inputBuf, pool, project, k);
	unsigned char* outputBuf = sub(inputBuf, openedImage, pool, project);
	return outputBuf;
}

// // Black top hat (residue find) operator using a 3x3 structuring element
// // = the difference between an image's closing and its original input image
EMSCRIPTEN_KEEPALIVE unsigned char* topHat3x3Black(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* closedImage = close3x3(inputBuf, pool, project, k);
	unsigned char* outputBuf = sub(closedImage, inputBuf, pool, project);
	return outputBuf;
}

// // Black top hat (residue find) operator using a 5x5 structuring element
// // = the difference between an image's closing and its original input image
EMSCRIPTEN_KEEPALIVE unsigned char* topHat5x5Black(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* closedImage = close5x5(inputBuf, pool, project, k);
	unsigned char* outputBuf = sub(closedImage, inputBuf, pool, project);
	return outputBuf;
}

// Determine whether an internal corner exists at a specified location via pattern matching
// TODO: Eliminate the code repetition - we can iterate through the structuring elements using pointer arithmetic
// Also, another way to do this is to copy the input image's neighborhood to an array and then compare it to the
// structuring element for equality
EMSCRIPTEN_KEEPALIVE bool findCornerExternal(unsigned char inputBuf[], Wasmcv* project, int loc) {
	std::array<int, 4> o = project->offsets._2x2;
	int hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2ecul.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2ecur.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2ecll.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2eclr.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	return false;
}

// Determine whether an external corner exists at a specified location via pattern matching
// TODO: Eliminate the code repetition - we can iterate through the structuring elements using pointer arithmetic
// Also, another way to do this is to copy the input image's neighborhood to an array and then compare it to the
// structuring element for equality
EMSCRIPTEN_KEEPALIVE bool findCornerInternal(unsigned char inputBuf[], Wasmcv* project, int loc) {
	std::array<int, 4> o = project->offsets._2x2;
	int hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2icul.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2icur.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2icll.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	hits = 0;
	for (int i = 0; i < 4; i += 1) {
		if (inputBuf[loc + o[i]] == project->se._2x2iclr.kernel[i]) {
			hits += 1;
		}
	}
	if (hits == 4) {
		return true;
	}
	return false;
}

// Find all internal and external corners via pattern matching
// TODO: Optimize! The last column and row of a given image are incapable of posessing a corner, so we
// should not bother operating on them
EMSCRIPTEN_KEEPALIVE std::vector<int> findAllCorners(unsigned char inputBuf[], Wasmcv* project) {
	std::vector<int> corners;
	int len = 0;
	for (int i = 3; i < project->size; i += 4) {
		if (findCornerInternal(inputBuf, project, i) || findCornerExternal(inputBuf, project, i)) {
			corners.push_back(i - 3);
		} 
	}
	return corners;
}

// Return the number of foreground objects as found via corner finding
EMSCRIPTEN_KEEPALIVE int countForegroundObjects(unsigned char inputBuf[], Wasmcv* project) {
	int e = 0;
	int i = 0;
	for (int i = 3; i < project->size; i += 4) {
		if (findCornerInternal(inputBuf, project, i)) e += 1;
		if (findCornerExternal(inputBuf, project, i)) i += 1;
	}
	return (e - i) / 4;
}

// Get conected components via recursive labeling (using 4-neighborhood) and return a segmentation map
// TODO: This algorithm fails on large images (even 640x480) due to depth of recursion - delete it entirely?
EMSCRIPTEN_KEEPALIVE int16_t* getConnectedComponentsRecursive(unsigned char inputBuf[], Wasmcv* project) {
	// Create the segmentation map and initialize it with 0s
	std::vector<int16_t> map(project->size, 0);
	// Translate the input buffer to -1 and copy it to the segmentation map
	for (int i = 3; i < project->size; i += 4) {
		map[i] = inputBuf[i] == 255 ? -1 : 0;
	}
	// Identifier for the label # we'll assign and increment to segments we find
	int label = 0;
	// Loop through the whole segmentation map looking for -1 values...
	for (int i = 3; i < project->size; i += 4) {
		if (map[i] == -1) {
			// If you find a pixel which has yet to be segmented and labeled,
			// first increment your label # to get ready to start labeling...
			label += 1;
			searchConnected(project, map, label, i);
		}
	}
	return map.data();
}

// Recursive function used as part of getConnectedComponentsRecursive()
// that searches 3x3 pixel neighborhoods for connected pixels
void searchConnected(Wasmcv* project, std::vector<int16_t>& map, int label, int offset) {
	// First, mark the origin pixel
	map[offset] = label;
	// Now loop through our neighbors
	for (int i = 0; i < 5; i += 1) {
		// First just check bounds
		if (offset + project->offsets._4n[i] >= 0 && offset + project->offsets._4n[i] < project->size) {
			// If we find a neighbor pixel that is connected, recurse the function
			// on that neighbor pixel
			if (map[offset + project->offsets._4n[i]] == -1) {
				searchConnected(project, map, label, offset + project->offsets._4n[i]);
			}
		}
	}
}

// Get connected components via union-find algorithm (using 4-neighborhood) and return a segmentation map
// A segmentation map is a 1:1 representation of an imagedata object but with region labels replacing positive pixel values
// TODO: Optimize! Should segmentation maps be pixel representations instead of byte representations?
// Should we implement run length encoding?
EMSCRIPTEN_KEEPALIVE std::vector<int> getConnectedComponents(unsigned char inputBuf[], Wasmcv* project) {
	// Init a global variable for our region labels
	int label = 1;
	// Init a disjoint set to keep track of our non-overlapping sets (which are discrete regions of connected pixels)
	int disjointSet[1200] = {0};
	//               ^ max number of segments we can label
	// Init a segmentation map which is what we'll return to the user
	std::vector<int> map(project->size, 0);
	// Pass 1/2 through our input image!
	for (int i = 3; i < project->size; i += 4) {
		// We only want to process this pixel if it is a foreground pixel
		if (inputBuf[i] == 255) {
			// First, we want to check if our north + west neighbors are already labeled with a value (indicating that they
			// belong to a set), and if so, we'll grab those values
			int16_t priorNeighborLabels[2] = {0};
			// Neighbor pixel to the north
			if (isInImageBounds(project, i + project->offsets._4n[0])) {
				priorNeighborLabels[0] = map[i + project->offsets._4n[0]]; // should i actually find the parent node here?
			}
 			// Neighbor pixel to the west
			if (isInImageBounds(project, i + project->offsets._4n[1]) && ((i - 3) / 4) % project->w != 0) { 
				priorNeighborLabels[1] = map[i + project->offsets._4n[1]]; // should i actually find the parent node here?
			}
			// Integer m will be the label for our output pixel...
			int m;
			// So if neither our north or west neighbor are already labeled with a value, then we figure that our current pixel
			// is not connected to any known pixels, so we'll proceed by getting ready to assign a brand new value
			if (priorNeighborLabels[0] == 0 && priorNeighborLabels[1] == 0) {
				m = label;
				label = label == 1199 ? 0 : label += 1;
				//                ^ Corresponds to max number of segments in our disjoint set data structure
			} else { 
			// Otherwise, if either our north or west neighbor has a label, we want to use the smaller of the two possible labels
			// But not a zero value! Zero is an init value and not a valid label value!
				if (priorNeighborLabels[1] > priorNeighborLabels[0]) std::swap(priorNeighborLabels[0], priorNeighborLabels[1]);
				m = priorNeighborLabels[0] == 0 ? priorNeighborLabels[1] : priorNeighborLabels[0];
			}
			// Now we have a label value we want to use, so let's assign it to our output pixel
			map[i] = m;	
			// Now we want to merge all of our neighbor pixels sets with the set of our current pixel (assuming that our current
			// pixel is not receiving exactly the same label as one of our neighbors
			// TODO: I think the bug is in here? For some reason, we're joining all of our sets together into one big set?
			for (int j = 0; j < 2; j += 1) {
				if (priorNeighborLabels[j] != 0 && priorNeighborLabels[j] != m) {
					constructUnion(m, priorNeighborLabels[j], disjointSet);
				}
			}
		}
	}
	// Pass 2/2 through our input image!
	for (int i = 3; i < project->size; i += 4) {
		// We only want to process this pixel if it is a foreground pixel
		if (inputBuf[i] == 255) {
			// Replace this pixel's label with the label of its parent node
			map[i] = findParent(map[i], disjointSet);
		}
	}
	return map;
}

// Implementation of find as part of the union-find algorithm
// Traverse a disjoint set tree to find the parent node of a given value
int findParent(int label, int disjointSet[]) {
	while (disjointSet[label] != 0) {	
		label = disjointSet[label];
	}
	return label;
}

// Implementation of union as part of the union-find algorithm
// Given two values, find the non-overlapping sets that they belong to and join those two sets
int* constructUnion(int labelX, int labelY, int disjointSet[]) {
	// Find the parent of the first label
	while (disjointSet[labelX] != 0) {
		labelX = disjointSet[labelX];
	}
	// Find the parent of the second label
	while (disjointSet[labelY] != 0) {
		labelY = disjointSet[labelY];
	}
	if (labelX != labelY) {
		disjointSet[labelY] = labelX;
	}
	return disjointSet;
}

// Turn a segmentation map into a psychedelic visualization of itself
// Returns a pointer to an image
EMSCRIPTEN_KEEPALIVE unsigned char* segMapToImage(std::vector<int> map, BufferPool* pool, Wasmcv* project) {
	unsigned char* outputBuf = pool->getNew();
	float r = float(rand()) / float(RAND_MAX);
	float g = float(rand()) / float(RAND_MAX);
	float b = float(rand()) / float(RAND_MAX);
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i - 3] = (unsigned char)(float(map[i]) * float(r));
		outputBuf[i - 2] = (unsigned char)(float(map[i]) * float(g));
		outputBuf[i - 1] = (unsigned char)(float(map[i]) * float(b));
		outputBuf[i] = 255;
	}
	return outputBuf;
}

// Create an image from a perimeter map
// Returns a pointer to an image
// TODO: Accept a Pixel object and draw the perimeter using it instead of hardcoded RGB green
EMSCRIPTEN_KEEPALIVE unsigned char* perimeterMapToImage(std::vector<int> map, BufferPool* pool, Wasmcv* project) {
	unsigned char* outputBuf = pool->getNew();
	for (int i = 0; i < project->size; i += 1) {
		outputBuf[i] = 0;
	}
	for (int i = 0; i < project->size; i += 1) {
		outputBuf[map[i]] = 102;
		outputBuf[map[i] + 1] = 255;
		outputBuf[map[i] + 2] = 51;
		outputBuf[map[i] + 3] = 255;
	}
	return outputBuf;
}

// Get the area of a region of pixels in a segmentation map
EMSCRIPTEN_KEEPALIVE int getRegionArea(std::vector<int> map, int label, Wasmcv* project) {
	int area = 0;
	for (int i = 3; i < project->size; i += 4) {
		if (map[i] == label) {
			area += 1;
		}
	}
	return area;
}

// Get the area for all regions of pixels in a segmentation map
// Returns an array of length 1200 (our hardcoded max number of image segments) where each element is
// the area of its corresponding region
EMSCRIPTEN_KEEPALIVE std::vector<int> getAllRegionAreas(std::vector<int> map, Wasmcv* project) {
 	std::vector<int> hist(1200, 0);
	for (int i = 3; i < project->size; i += 4) {
		hist[map[i]] += 1;
	}
	return hist;
}

// Get region centroid for a labeled region in a segmentation map
// vector[0] == x coord
// vector[1] == y coord
EMSCRIPTEN_KEEPALIVE std::vector<int> getRegionCentroid(std::vector<int> map, int label, Wasmcv* project) {
	std::vector<int> centroid;
	long accumulatedX = 0;
	long accumulatedY = 0;
	int area = 0;
	for (int i = 3; i < project->size; i += 4) {
		if (map[i] == label) {
			int pixelOffset = (i - 3) / 4;
			int x = pixelOffset % project->w;
	 		int y = pixelOffset / project->w;
	 		accumulatedX += x;
	 		accumulatedY += y;
	 		area += 1;
		}
	}
	centroid.push_back(accumulatedX / area);
	centroid.push_back(accumulatedY / area);
	return centroid;
}

// Get region centroids for all regions of pixels in a segmentation map
// Returns a vector of 1200 elements (our hardcoded max number of image segments) 
// where each element corresponds to a region label and holds the value of an imagedata offset 
// representing the centroid of that region label
// TODO: This is needlessly converting from offset to vec2 and back again
EMSCRIPTEN_KEEPALIVE std::vector<int> getAllRegionCentroids(std::vector<int> map, int areaThresh, Wasmcv* project) {
	// Create an array of a length = our max number of image segments
	std::vector<int> centroids(1200, 0);
	long accumulatedX[1200] = {0};
	long accumulatedY[1200] = {0};
	int area[1200] = {0};
	for (int i = 3; i < project->size; i += 4) {
		// We loop through the image, accumulating the area and X/Y positions for each pixel in every labeled region
		if (map[i] != 0) {
			int pixelOffset = (i - 3) / 4;
			int x = pixelOffset % project->w;
	 		int y = pixelOffset / project->w;
	 		accumulatedX[map[i]] += x;
	 		accumulatedY[map[i]] += y;
			area[map[i]] += 1;
		}
	}
	// Now we loop through our returnable array, calculating the centroid for every labeled region
	for (int i = 0; i < 1200; i += 1) {
		if (area[i] > areaThresh) {
			int avgX = accumulatedX[i] / area[i];
			int avgY = accumulatedY[i] / area[i];
			int offset = avgY * project->w * 4 + (avgX * 4);
			centroids[i] = offset;
		}
	}
	return centroids;
}

// Extract a region from a segmentation map
// Returns a vector of imgdata offsets
EMSCRIPTEN_KEEPALIVE std::vector<int> getRegion(std::vector<int> map, int label, Wasmcv* project) {
	// Init a vector to hold our returnable pixel locations
	std::vector<int> region;
	for (int i = 3; i < project->size; i += 4) {
		if (map[i] == label) {
			region.push_back(i); // We're pushing in the value of the alpha byte of each pixel - be careful!
		}
	}
	return region;
}

// Get perimeter of a labeled region in a segmentation map using the 4-connected model
// Returns a vector of imgdata offsets
EMSCRIPTEN_KEEPALIVE std::vector<int> getRegionPerimeter(std::vector<int> map, int label, Wasmcv* project) {
	// Init a vector to hold our returnable perimeter locations
	std::vector<int> perimeter;
	// First we need to collect offset locations for every pixel in the given region
	auto region = getRegion(map, label, project);
	// Now we loop through every pixel in the given region 
	for (int i = 0; i < region.size(); i += 1) {
		// For the given pixel, let's get the set of labels belonging to its neighbors
		std::vector<int> neighborLabels;
		for (int j = 0; j < 5; j += 1) {
			if (isInImageBounds(project, region[i] + project->offsets._4n[j])) {
				neighborLabels.push_back(map[region[i] + project->offsets._4n[j]]);
			}
		}
		// Now check if all of the set of labels belonging to our pixel's neighbors are
		// the same label as our pixel
		int value = label;
		for (int j = 0; value == label && j < neighborLabels.size(); j += 1) {
			value = neighborLabels[j];
		}
		// So if all of our neighbor labels == our pixel label, then our pixel is
		// surrounded by more of its kind and is NOT a border pixel
		if (value != label) {
			perimeter.push_back(region[i] - 3); // We're pushing in the red byte of each pixel - be careful!
		}
	}
	return perimeter;
}

// Get bounding box coords for a given map of perimeter pixels
// Returns a vector of 4 elements: [x, y, w, h]
EMSCRIPTEN_KEEPALIVE std::vector<int> getBoundingBox(std::vector<int> perimeterMap, Wasmcv* project) {
	// Get initial values for the min/max
	auto vec2 = offsetToVec2(perimeterMap[0], project);
	int xMax, xMin = vec2[0];
	int yMax, yMin = vec2[1];
	// Init the returnable vector
	std::vector<int> boundingBox(4);;
	// Loop through our perimeter pixels evaluating for min/max
	for (int i = 1; i < perimeterMap.size(); i += 1) {
		auto vec2 = offsetToVec2(perimeterMap[i], project);
	 	if (vec2[0] > xMax) xMax = vec2[0];
	 	if (vec2[0] < xMin) xMin = vec2[0];
	 	if (vec2[1] > yMax) yMax = vec2[1];
	 	if (vec2[1] < yMin) yMin = vec2[1];
	}
	boundingBox[0] = xMin;
	boundingBox[1] = yMin;
	boundingBox[2] = xMax - xMin;
	boundingBox[3] = yMax - yMin;
	return boundingBox;
}

// Get extremal axis length
EMSCRIPTEN_KEEPALIVE int getExtremalAxisLength(int x1, int y1, int x2, int y2) {
	float angle = std::atan2(y2 - y1, x2 - x1);
	float increment = angle < 0.785398 ? 1 / std::cos(angle) : 1 / std::sin(angle);
	return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2) + increment);
}

// Get second order row moment
// TODO: check the math - isn't this supposed to be shape and translation invariant?
EMSCRIPTEN_KEEPALIVE float getSecondOrderRowMoment(std::vector<int> map, int label, Wasmcv* project) {
	auto region = getRegion(map, label, project);
	std::vector<int> yVals;
	float yAcc = 0;
	for (int i = 0; i < region.size(); i += 1) {
		auto vec2 = offsetToVec2(region[i], project);
		yVals.push_back(vec2[1]);
		yAcc += vec2[1];
	}
	float mean = yAcc / float(region.size());
	float sum = 0;
	for (int i = 0; i < region.size(); i += 1) {
		sum += std::pow(float(yVals[i]) - mean, 2);
	}
	float secondOrderRowMoment = sum / float(region.size());
	return secondOrderRowMoment;
}

// Get second order column moment
// TODO: check the math - isn't this supposed to be shape and translation invariant?
EMSCRIPTEN_KEEPALIVE float getSecondOrderColumnMoment(std::vector<int> map, int label, Wasmcv* project) {
	auto region = getRegion(map, label, project);
	std::vector<int> xVals;
	float xAcc = 0;
	for (int i = 0; i < region.size(); i += 1) {
		auto vec2 = offsetToVec2(region[i], project);
		xVals.push_back(vec2[0]);
		xAcc += vec2[0];
	}
	float mean = xAcc / float(region.size());
	float sum = 0;
	for (int i = 0; i < region.size(); i += 1) {
		sum += std::pow(float(xVals[i]) - mean, 2);
	}
	float secondOrderColumnMoment = sum / float(region.size());
	return secondOrderColumnMoment;
}

// Get second order mixed moment
// TODO: check the math - isn't this supposed to be shape and translation invariant?
EMSCRIPTEN_KEEPALIVE float getSecondOrderMixedMoment(std::vector<int> map, int label, Wasmcv* project) {
	auto region = getRegion(map, label, project);
	std::vector<int> xVals, yVals;
	float xAcc = 0, yAcc = 0;
	for (int i = 0; i < region.size(); i += 1) {
		auto vec2 = offsetToVec2(region[i], project);
		xVals.push_back(vec2[0]);
		yVals.push_back(vec2[1]);
		xAcc += vec2[0];
		yAcc += vec2[1];
	}
	float xMean = xAcc / float(region.size());
	float yMean = yAcc / float(region.size());
	float sum = 0;
	for (int i = 0; i < region.size(); i += 1) {
		sum += (yVals[i] - yMean) * (xVals[i] - xMean);
	}
	float secondOrderMixedMoment = sum / float(region.size());
	return secondOrderMixedMoment;
}

#ifdef __cplusplus
}
#endif