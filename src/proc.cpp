#ifdef __cplusplus
extern "C" {
#endif

// Return neighbor pixel offsets as an array
// Note that the identity input pixel offset is not returned!
// 0 1 2
// 3 X 5
// 6 7 8   = [0, 1, 2, 3, 4, 5, 6, 7]
std::array<int, 8> getNeighborOffsets(int w) {
	int p0 = -w * 4 - 4;
	int p1 = -w * 4;
	int p2 = -w * 4 + 4;
	int p3 = -4;
	int p5 = 4;
	int p6 = w * 4 - 4;
	int p7 = w * 4;
	int p8 = w * 4 + 4;
	std::array<int, 8> offsets = {p0, p1, p2, p3, p5, p6, p7, p8};
	return offsets;
}

// Get sigma (sum of neighbor pixel values) for a binary thresholded image
int getSigma(unsigned char inputBuf[], int p0, int w) {
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

// Set all of an image buffer's pixel values to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* level(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = r;
		outputBuf[i + 1] = g;
		outputBuf[i + 2] = b;
		outputBuf[i + 3] = a;
	}
	return outputBuf;
}

// Set all of an image buffer's r channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelR(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char r) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = r;
	}
	return outputBuf;
}

// Set all of an image buffer's g channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelG(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char g) {
	for (int i = 1; i < size; i += 4) {
		outputBuf[i] = g;
	}
	return outputBuf;
}

// Set all of an image buffer's b channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelB(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char b) {
	for (int i = 2; i < size; i += 4) {
		outputBuf[i] = b;
	}
	return outputBuf;
}

// Set all of an image buffer's a channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelA(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char a) {
	for (int i = 3; i < size; i += 4) {
		outputBuf[i] = a;
	}
	return outputBuf;
}

// Invert an image
EMSCRIPTEN_KEEPALIVE unsigned char* invert(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = 255 - inputBuf[i];
		outputBuf[i + 1] = 255 - inputBuf[i + 1];
		outputBuf[i + 2] = 255 - inputBuf[i + 2];
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
	}
	return outputBuf;
}

// Shift an image left
EMSCRIPTEN_KEEPALIVE unsigned char* shiftLeft(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = inputBuf[i + d * 4];
		outputBuf[i + 1] = inputBuf[i + 1 + d * 4];
		outputBuf[i + 2] = inputBuf[i + 2 + d * 4];
		outputBuf[i + 3] = inputBuf[i + 3 + d * 4];
	}
	return outputBuf;
}

// Shift an image right
EMSCRIPTEN_KEEPALIVE unsigned char* shiftRight(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = inputBuf[i - d * 4];
		outputBuf[i + 1] = inputBuf[i + 1 - d * 4];
		outputBuf[i + 2] = inputBuf[i + 2 - d * 4];
		outputBuf[i + 3] = inputBuf[i + 3 - d * 4];
	}
	return outputBuf;
}

// Shift an image up
EMSCRIPTEN_KEEPALIVE unsigned char* shiftUp(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = inputBuf[i + d * w * 4];
		outputBuf[i + 1] = inputBuf[i + 1 + d * w * 4];
		outputBuf[i + 2] = inputBuf[i + 2 + d * w * 4];
		outputBuf[i + 3] = inputBuf[i + 3 + d * w * 4];
	}
	return outputBuf;
}

// Shift an image down
EMSCRIPTEN_KEEPALIVE unsigned char* shiftDown(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = inputBuf[i - d * w * 4];
		outputBuf[i + 1] = inputBuf[i + 1 - d * w * 4];
		outputBuf[i + 2] = inputBuf[i + 2 - d * w * 4];
		outputBuf[i + 3] = inputBuf[i + 3 - d * w * 4];
	}
	return outputBuf;
}

// Lighten an image
EMSCRIPTEN_KEEPALIVE unsigned char* lighten(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int v) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = std::clamp(inputBuf[i] + v, 0, 255);
		outputBuf[i + 1] = std::clamp(inputBuf[i + 1] + v, 0, 255);
		outputBuf[i + 2] = std::clamp(inputBuf[i + 2] + v, 0, 255);
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
	}
	return outputBuf;
}

// Darken an image
EMSCRIPTEN_KEEPALIVE unsigned char* darken(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int b) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = std::clamp(inputBuf[i] - b, 0, 255);
		outputBuf[i + 1] = std::clamp(inputBuf[i + 1] - b, 0, 255);
		outputBuf[i + 2] = std::clamp(inputBuf[i + 2] - b, 0, 255);
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
	}
	return outputBuf;
}

// Contrast stretch
EMSCRIPTEN_KEEPALIVE unsigned char* contrast(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int g, int b) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = std::clamp(inputBuf[i] * g + b, 0, 255);
		outputBuf[i + 1] = std::clamp(inputBuf[i + 1] * g + b, 0, 255);
		outputBuf[i + 2] = std::clamp(inputBuf[i + 2] * g + b, 0, 255);
		outputBuf[i + 3] = std::clamp(inputBuf[i + 3] * g + b, 0, 255);
	}
	return outputBuf;
}

// Convert to simulated grayscale colorspace
EMSCRIPTEN_KEEPALIVE unsigned char* toGrayscale(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	for (int i = 0; i < size; i += 4) {
		int luma = inputBuf[i] * 0.2126 + inputBuf[i + 1] * 0.7152 + inputBuf[i + 2] * 0.0722;
		outputBuf[i] = 0;
		outputBuf[i + 1] = 0;
		outputBuf[i + 2] = 0;
		outputBuf[i + 3] = 255 - luma;
	}
	return outputBuf;
}

// Threshold a grayscale image to binary
EMSCRIPTEN_KEEPALIVE unsigned char* toBinary(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int t) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = 0;
		outputBuf[i + 1] = 0;
		outputBuf[i + 2] = 0;
		outputBuf[i + 3] = inputBuf[i + 3] > t ? 255 : 0;
	}
	return outputBuf;
}

// Erode (shrink) dark binary thresholded objects
EMSCRIPTEN_KEEPALIVE unsigned char* erode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = getSigma(inputBuf, i, w);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = sigma < 2040 ? outputBuf[i] = 0 : outputBuf[i] = inputBuf[i];
	}
	return outputBuf;
}

// Dilate (grow) dark binary thresholded objects
EMSCRIPTEN_KEEPALIVE unsigned char* dilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = getSigma(inputBuf, i, w);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = sigma > 0 ? outputBuf[i] = 255 : outputBuf[i] = inputBuf[i];
	}
	return outputBuf;
}

// Find binary edges
EMSCRIPTEN_KEEPALIVE unsigned char* findEdges(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = getSigma(inputBuf, i, w);
		outputBuf[i] = sigma == 2040 ? outputBuf[i] = 0 : outputBuf[i] = inputBuf[i];
	}
	return outputBuf;
}

// Remove salt and pepper noise (binary values 0 and 1)
EMSCRIPTEN_KEEPALIVE unsigned char* removeSaltPepper(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size) {
	for (int i = 3; i < size; i += 4) {
		int sigma = getSigma(inputBuf, i, w);
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
EMSCRIPTEN_KEEPALIVE unsigned char* suppressNoise(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	int k[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	return conv3(inputBuf, outputBuf, w, size, k, 9);
}

// Noise suppression by better neighborhood averaging
EMSCRIPTEN_KEEPALIVE unsigned char* suppressNoisePlus(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	int k[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
	return conv3(inputBuf, outputBuf, w, size, k, 16);
}

#ifdef __cplusplus
}
#endif