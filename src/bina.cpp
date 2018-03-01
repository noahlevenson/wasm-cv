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

#ifdef __cplusplus
}
#endif