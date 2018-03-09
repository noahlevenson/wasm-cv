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
// If dilation finds a black input pixel, then it ORs the entire structuring kernel, centered over the input pixel, to the output image
// The effect is that black pixel regions "grow" in approximately the shape of the structuring kernel
// TODO: Optimize this - there's a faster implementation
unsigned char* kDilate3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement3x3 k) {
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
// If dilations finds a black input pixel, then it ORs the entire structuring kernel, centered over the input pixel, to the output image
// The effect is that black pixel regions "grow" in approximately the shape of the structuring kernel
// TODO: Optimize this - there's a faster implementation
unsigned char* kDilate5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement5x5 k) {
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
// We center the structuring element over each input pixel and check whether every black pixel in the structuring element corresponds
// to a black pixel in the image beneath it. If yes, then we OR the center value of our structuring element with our centered input pixel
// The effect is that white pixel regions "grow" in approximately the shape of the structuring kernel
// TODO: Instead of ORing output pixel with structuring kernel pixel per the spec, we're simply making output pixel = input pixel...
// I think we get this for free as a result of predetermining that if structuring kernel is completely contained, and input pixel = 255,
// then structuring kernel must also = 255...
// case a: i = 255, k = 255 -- possible complete containment, output pixel should = 255
// case b: i = 255, k = 0 -- possible complete containment, output pixel should = 255
// case c: i = 0, k = 255 -- no complete containment, output pixel should = 0
// case d: i = 0, k = 0 -- possible complete containment, output pixel should = 0
// TODO: Optimize this - there's a faster implementation
unsigned char* kErode3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement3x3 k) {
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
unsigned char* kErode5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement5x5 k) {
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
// Finds neighborhoods that are entirely black and replaces their p0 with a white value
// the result is that deep black "fill" areas are turned white, while the edges of those fill areas are preserved as black
EMSCRIPTEN_KEEPALIVE unsigned char* findEdges(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
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
EMSCRIPTEN_KEEPALIVE unsigned char* close3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = kDilate3x3(inputBuf, tempBuf, project, k);
	outputBuf = kErode3x3(tempBuf, outputBuf, project, k);
	delete [] tempBuf;
	return outputBuf;
}

// Binary morphological closing using a 5x5 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* close5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = kDilate5x5(inputBuf, tempBuf, project, k);
	outputBuf = kErode5x5(tempBuf, outputBuf, project, k);
	delete [] tempBuf;
	return outputBuf;
}

// Binary morphological opening using a 3x3 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* open3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = kErode3x3(inputBuf, tempBuf, project, k);
	outputBuf = kDilate3x3(tempBuf, outputBuf, project, k);
	delete [] tempBuf;
	return outputBuf;
}

// Binary morphological opening using a 3x3 structuring element
EMSCRIPTEN_KEEPALIVE unsigned char* open5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = kErode5x5(inputBuf, tempBuf, project, k);
	outputBuf = kDilate5x5(tempBuf, outputBuf, project, k);
	delete [] tempBuf;
	return outputBuf;
}

// Subtract an image from an image
EMSCRIPTEN_KEEPALIVE unsigned char* sub(unsigned char inputBufA[], unsigned char inputBufB[], unsigned char outputBuf[], Wasmcv* project) {
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
EMSCRIPTEN_KEEPALIVE unsigned char* topHat3x3White(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = open3x3(inputBuf, tempBuf, project, k);
	outputBuf = sub(inputBuf, tempBuf, outputBuf, project);
	delete [] tempBuf;
	return outputBuf;
}

// White top hat (residue find) operator using a 5x5 structuring element
// = the difference between an input image and its opening 
EMSCRIPTEN_KEEPALIVE unsigned char* topHat5x5White(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = open5x5(inputBuf, tempBuf, project, k);
	outputBuf = sub(inputBuf, tempBuf, outputBuf, project);
	delete [] tempBuf;
	return outputBuf;
}

// Black top hat (residue find) operator using a 3x3 structuring element
// = the difference between an image's closing and its original input image
EMSCRIPTEN_KEEPALIVE unsigned char* topHat3x3Black(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement3x3 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = close3x3(inputBuf, tempBuf, project, k);
	outputBuf = sub(tempBuf, inputBuf, outputBuf, project);
	delete [] tempBuf;
	return outputBuf;
}

// Black top hat (residue find) operator using a 5x5 structuring element
// = the difference between an image's closing and its original input image
EMSCRIPTEN_KEEPALIVE unsigned char* topHat5x5Black(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, BinaryStructuringElement5x5 k) {
	unsigned char* tempBuf = new unsigned char[project->size];
	tempBuf = close5x5(inputBuf, tempBuf, project, k);
	outputBuf = sub(tempBuf, inputBuf, outputBuf, project);
	delete [] tempBuf;
	return outputBuf;
}

// Determine whether an internal corner exists at a specified location via pattern matching
// TODO: Eliminate the code repetition - we can iterate through the structuring elements using pointer arithmetic
// also, another way to do this is to copy the input image's neighborhood to an array and then compare it to the
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
// also, another way to do this is to copy the input image's neighborhood to an array and then compare it to the
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
// Returns a heap pointer (js type UInt32Array) to a wasm array of imgdata offsets where element 0 = the length of the array
// TODO: Optimization - it may be faster to create a very large raw array and simply not use all of it
// ALSO TODO: The last column and row of a given image are incapable of posessing a corner, so we
// should not bother operating on them
EMSCRIPTEN_KEEPALIVE uint32_t* findAllCorners(unsigned char inputBuf[], Wasmcv* project) {
	std::vector<uint32_t> corners;
	corners.push_back(0);
	int len = 0;
	for (int i = 3; i < project->size; i += 4) {
		if (findCornerInternal(inputBuf, project, i) || findCornerExternal(inputBuf, project, i)) {
			corners.push_back(i - 3);
			len += 1;
		} 
	}
	corners[0] = len;
	return corners.data();
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
// TODO: is int16_t the most efficient data type? For wasm-cv functions that
// return arrays, we should consider standardizing their return signature
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
// TODO: is int16_t the most efficient data type? For wasm-cv functions that
// return arrays, we should consider standardizing their return signature
// ALSO TODO: It can get awfully slow depending on what type of image it's working on - how can we optimize?
EMSCRIPTEN_KEEPALIVE int16_t* getConnectedComponents(unsigned char inputBuf[], Wasmcv* project) {
	// Init a global variable for our region labels
	int label = 1;
	// Init a disjoint set to keep track of our non-overlapping sets (which are discrete regions of connected pixels)
	int disjointSet[1200] = {0};
	//               ^ max number of segments we can label
	// Init a segmentation map which is what we'll return to the user
	std::vector<int16_t> map(project->size, 0);
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
	return map.data();
}

// Implementation of find as part of the union-find algorithm
// Traverse a disjoint set data strcture to find the parent node of a given value
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

// Get the area of a region of pixels in a segmentation map
// Returns the area as an integer
EMSCRIPTEN_KEEPALIVE int getRegionArea(int16_t* map, int16_t label, Wasmcv* project) {
	int area = 0;
	for (int i = 3; i < project->size; i += 4) {
		if (map[i] == label) {
			area += 1;
		}
	}
	return area;
}

// Get the area for all regions of pixels in a segmentation map
// Returns an array of length 1200 (our hardcoded max number of image segments) where each index is
// the area of its corresponding region
EMSCRIPTEN_KEEPALIVE uint32_t* getAllRegionAreas(int16_t* map, Wasmcv* project) {
 	std::vector<uint32_t> hist(1200, 0);
	for (int i = 3; i < project->size; i += 4) {
		hist[map[i]] += 1;
	}
	return hist.data();
}

// Get region centroid for a labeled region in a segmentation map
// Returns a heap pointer (js type UInt32Array) to a wasm array of size 2
// index[0] == x coord
// index[1] == y coord
EMSCRIPTEN_KEEPALIVE uint32_t* getRegionCentroid(int16_t* map, int16_t label, Wasmcv* project) {
	std::vector<uint32_t> centroid;
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
	return centroid.data();
}

// Get region centroids for all regions of pixels in a segmentation map
// Returns a heap pointer (js type UInt32Array) to a wasm array of size 1200 
// (our hardcoded max number of image segments) where each index corresponds to a region label
// and each index holds the value of an imagedata offset representing the centroid of that region label
// TODO: This is VERY inefficient because it converts from offset to x/y coords and then back again...
// we should figure out a way to pass x/y coords back to javascript...
EMSCRIPTEN_KEEPALIVE uint32_t* getAllRegionCentroids(int16_t* map, int areaThresh, Wasmcv* project) {
	// Create an array of a length = our max number of image segments
	std::vector<uint32_t> centroids(1200, 0);
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
	return centroids.data();
}

// Get perimeter of a labeled region in a segmentation map using the 4-connected model
// Returns a heap pointer (js type UInt32Array) to a wasm array of imgdata offsets where element 0 = the length of the array
EMSCRIPTEN_KEEPALIVE uint32_t* getRegionPerimeter(int16_t* map, int16_t label, Wasmcv* project) {
	// Init a vector to hold our returnable perimeter locations
	std::vector<uint32_t> perimeter(1, 0);
	// First we need to collect offset locations for every pixel in the given region
	std::vector<int32_t> region;
	for (int i = 3; i < project->size; i += 4) {
		if (map[i] == label) {
			region.push_back(i); // We're pushing in the A byte of each pixel - be careful!
		}
	}
	// Now we loop through every pixel in the given region 
	for (int i = 0; i < region.size(); i += 1) {
		// For the given pixel, let's get the set of labels belonging to its neighbors
		std::vector<int16_t> neighborLabels;
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
			perimeter.push_back(region[i] - 3); // We're pushing in the R byte of each pixel - watch this!
		}
	}
	perimeter[0] = perimeter.size() - 1;
	return perimeter.data();
}


#ifdef __cplusplus
}
#endif