#ifdef __cplusplus
extern "C" {
#endif

// Set all of an image buffer's pixel values to a specified values
EMSCRIPTEN_KEEPALIVE unsigned char* level(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, Pixel p) {
	for (int i = 0; i < size; i += 4) {
		outputBuf[i] = p.r;
		outputBuf[i + 1] = p.g;
		outputBuf[i + 2] = p.b;
		outputBuf[i + 3] = p.a;
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

#ifdef __cplusplus
}
#endif