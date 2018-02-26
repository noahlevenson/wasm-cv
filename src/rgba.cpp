#ifdef __cplusplus
extern "C" {
#endif

// Set all of an image buffer's pixel values to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* level(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = r;
		outputBuf[i + 1] = g;
		outputBuf[i + 2] = b;
		outputBuf[i + 3] = a;
	}
	return outputBuf;
}

// Set all of an image buffer's r channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelR(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char r) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = r;
	}
	return outputBuf;
}

// Set all of an image buffer's g channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelG(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char g) {
	for (int i = 1; i < project->size; i += 4) {
		outputBuf[i] = g;
	}
	return outputBuf;
}

// Set all of an image buffer's b channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelB(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char b) {
	for (int i = 2; i < project->size; i += 4) {
		outputBuf[i] = b;
	}
	return outputBuf;
}

// Set all of an image buffer's a channel to a specified value
EMSCRIPTEN_KEEPALIVE unsigned char* levelA(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char a) {
	for (int i = 3; i < project->size; i += 4) {
		outputBuf[i] = a;
	}
	return outputBuf;
}

// Invert an RGBA image
EMSCRIPTEN_KEEPALIVE unsigned char* invert(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = 255 - inputBuf[i];
		outputBuf[i + 1] = 255 - inputBuf[i + 1];
		outputBuf[i + 2] = 255 - inputBuf[i + 2];
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
	}
	return outputBuf;
}

// Shift an RGBA image left
EMSCRIPTEN_KEEPALIVE unsigned char* shiftLeft(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = inputBuf[i + d * 4];
		outputBuf[i + 1] = inputBuf[i + 1 + d * 4];
		outputBuf[i + 2] = inputBuf[i + 2 + d * 4];
		outputBuf[i + 3] = inputBuf[i + 3 + d * 4];
	}
	return outputBuf;
}

// Shift an RGBA image right
EMSCRIPTEN_KEEPALIVE unsigned char* shiftRight(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = inputBuf[i - d * 4];
		outputBuf[i + 1] = inputBuf[i + 1 - d * 4];
		outputBuf[i + 2] = inputBuf[i + 2 - d * 4];
		outputBuf[i + 3] = inputBuf[i + 3 - d * 4];
	}
	return outputBuf;
}

// Shift an RGBA image up
EMSCRIPTEN_KEEPALIVE unsigned char* shiftUp(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = inputBuf[i + d * project->w * 4];
		outputBuf[i + 1] = inputBuf[i + 1 + d * project->w * 4];
		outputBuf[i + 2] = inputBuf[i + 2 + d * project->w * 4];
		outputBuf[i + 3] = inputBuf[i + 3 + d * project->w * 4];
	}
	return outputBuf;
}

// Shift an RGBA image down
EMSCRIPTEN_KEEPALIVE unsigned char* shiftDown(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = inputBuf[i - d * project->w * 4];
		outputBuf[i + 1] = inputBuf[i + 1 - d * project->w * 4];
		outputBuf[i + 2] = inputBuf[i + 2 - d * project->w * 4];
		outputBuf[i + 3] = inputBuf[i + 3 - d * project->w * 4];
	}
	return outputBuf;
}

// Lighten an RGBA image
EMSCRIPTEN_KEEPALIVE unsigned char* lighten(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int v) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = std::clamp(inputBuf[i] + v, 0, 255);
		outputBuf[i + 1] = std::clamp(inputBuf[i + 1] + v, 0, 255);
		outputBuf[i + 2] = std::clamp(inputBuf[i + 2] + v, 0, 255);
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
	}
	return outputBuf;
}

// Darken an RGBA image
EMSCRIPTEN_KEEPALIVE unsigned char* darken(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int b) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = std::clamp(inputBuf[i] - b, 0, 255);
		outputBuf[i + 1] = std::clamp(inputBuf[i + 1] - b, 0, 255);
		outputBuf[i + 2] = std::clamp(inputBuf[i + 2] - b, 0, 255);
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
	}
	return outputBuf;
}

// Contrast stretch an RGBA image
EMSCRIPTEN_KEEPALIVE unsigned char* contrast(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int g, int b) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = std::clamp(inputBuf[i] * g + b, 0, 255);
		outputBuf[i + 1] = std::clamp(inputBuf[i + 1] * g + b, 0, 255);
		outputBuf[i + 2] = std::clamp(inputBuf[i + 2] * g + b, 0, 255);
		outputBuf[i + 3] = std::clamp(inputBuf[i + 3] * g + b, 0, 255);
	}
	return outputBuf;
}

// Convert an RGBA image to simulated grayscale colorspace
EMSCRIPTEN_KEEPALIVE unsigned char* toGrayscale(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	for (int i = 0; i < project->size; i += 4) {
		int luma = inputBuf[i] * 0.2126 + inputBuf[i + 1] * 0.7152 + inputBuf[i + 2] * 0.0722;
		outputBuf[i] = 0;
		outputBuf[i + 1] = 0;
		outputBuf[i + 2] = 0;
		outputBuf[i + 3] = 255 - luma;
	}
	return outputBuf;
}

// Median filter a color image
EMSCRIPTEN_KEEPALIVE unsigned char* medianRGBA(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	std::array<int, 9> o = project->offsets._3x3;
	int histR[256] = {0};
	int histG[256] = {0};
	int histB[256] = {0};
	for (int i = 0; i < project->size; i += 4) {
		for (int j = 0; j < 9; j += 1) {
			histR[inputBuf[i + o[j]]] += 1;
			histG[inputBuf[i + 1 + o[j]]] += 1;
			histB[inputBuf[i + 2 + o[j]]] += 1;
		}
		int r = 0;
		int sum = 0;
		while (sum < 5) {
			sum += histR[r];
			r += 1;
		}
		int g = 0;
		sum = 0;
		while (sum < 5) {
			sum += histG[g];
			g += 1;
		}
		int b = 0;
		sum = 0;
		while (sum < 5) {
			sum += histB[b];
			b += 1;
		}
		outputBuf[i] = r - 1;
		outputBuf[i + 1] = g - 1;
		outputBuf[i + 2] = b - 1;
		outputBuf[i + 3] = inputBuf[i + 3]; // Preserve alpha
		for (int j = 0; j < 9; j += 1) {
			histR[inputBuf[i + o[j]]] = 0;
			histG[inputBuf[i + 1 + o[j]]] = 0;
			histB[inputBuf[i + 2 + o[j]]] = 0;
		}
	}
	return outputBuf;
}

#ifdef __cplusplus
}
#endif