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
		if (hits == k.positives) {
			outputBuf[i] = inputBuf[i] == 255 || k.kernel[k.p0] == 255 ? 255 : 0;
		} else {
			outputBuf[i] = 0;
		}
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
		if (hits == k.positives) {
			outputBuf[i] = inputBuf[i] == 255 || k.kernel[k.p0] == 255 ? 255 : 0;
		} else {
			outputBuf[i] = 0;
		}
	}
	return outputBuf;
}

// Find edges (simple)
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
		// TODO: GET THIS MATH RIGHT
		std::cout << int(inputBufA[i] - inputBufB[i]) << std::endl;
	}
	return outputBuf;
}

// White top hat (residue find) operator


#ifdef __cplusplus
}
#endif