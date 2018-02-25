#ifdef __cplusplus
extern "C" {
#endif

// Get sum of neighbor pixel values for a binary thresholded image
// Note that this is NEIGHBORS ONLY, does not include p0 value
int sumNeighbors(unsigned char inputBuf[], int p0, int w) {
	return inputBuf[p0 - 4 - w * 4] + inputBuf[p0 - w * 4] + inputBuf[p0 + 4 - w * 4] + inputBuf[p0 - 4] + inputBuf[p0 + 4] + 
		inputBuf[p0 - 4 + w * 4] + inputBuf[p0 + w * 4] + inputBuf[p0 + 4 + w * 4];
}

// Convolve an binary thresholded image with a 3x3 kernel
unsigned char* conv3(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size, int k[], int norm) {
	for (int i = 3; i < size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = ((inputBuf[i - 4 - w * 4] * k[0]) + (inputBuf[i - w * 4] * k[1]) + (inputBuf[i + 4 - w * 4] * k[2]) + 
			(inputBuf[i - 4] * k[3]) + (inputBuf[i] * k[4]) + (inputBuf[i + 4] * k[5]) + (inputBuf[i- 4 + w * 4] * k[6]) + 
				(inputBuf[i + w * 4] * k[7]) + (inputBuf[i + 4 + w * 4] * k[8])) / norm;
	}
	return outputBuf;
}

// Simple dilate dark objects 
EMSCRIPTEN_KEEPALIVE unsigned char* dilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, w);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = sigma > 0 ? 255 : inputBuf[i];
	}
	return outputBuf;
}

// Simple erode dark objects
EMSCRIPTEN_KEEPALIVE unsigned char* erode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, w);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = sigma < 2040 ? 0 : inputBuf[i];
	}
	return outputBuf;
}

// Binary dilation using a structuring kernel
EMSCRIPTEN_KEEPALIVE unsigned char* kDilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	std::array<unsigned char, 9> k = {255, 255, 255, 255, 255, 255, 255, 255, 255};
	std::array<int, 9> o = getNeighborOffsets(w);
	for (int i = 3; i < size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		for (int j = 0; j < 9; j += 1) { // This is inefficient - we should exit the loop and set our output pixel value as soon as we find one hit
			outputBuf[i] = k[j] == 255 && inputBuf[i + o[j]] ? 255 : inputBuf[i];	
		}
	}
	return outputBuf;
}
// Binary erosion using a structuring kernel
EMSCRIPTEN_KEEPALIVE unsigned char* kErode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	std::array<unsigned char, 9> k = {255, 255, 255, 255, 255, 255, 255, 255, 255};
	std::array<int, 9> o = getNeighborOffsets(w);
	for (int i = 3; i < size; i += 4) {
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		std::array<unsigned char, 9> n = {0};
		for (int j = 0; j < 9; j += 1) {
			n[j] = inputBuf[i + o[j]];
		} 
		outputBuf[i] = k == n ? 255 : 0;
	}
	return outputBuf;
}

// Find edges (simple)
EMSCRIPTEN_KEEPALIVE unsigned char* findEdges(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, w);
		outputBuf[i] = sigma == 2040 ? 0 : inputBuf[i];
	}
	return outputBuf;
}

// Remove salt and pepper noise (simple)
EMSCRIPTEN_KEEPALIVE unsigned char* deSaltPepper(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = sumNeighbors(inputBuf, i, w);
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
EMSCRIPTEN_KEEPALIVE unsigned char* boxBlur(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	int k[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	return conv3(inputBuf, outputBuf, w, size, k, 9);
}

// Improved noise suppression by neighborhood averaging (Gaussian approximation)
EMSCRIPTEN_KEEPALIVE unsigned char* gaussianApprox(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	int k[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
	return conv3(inputBuf, outputBuf, w, size, k, 16);
}

#ifdef __cplusplus
}
#endif