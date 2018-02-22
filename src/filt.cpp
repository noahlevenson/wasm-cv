#ifdef __cplusplus
extern "C" {
#endif

// Median filter a grayscale image
EMSCRIPTEN_KEEPALIVE unsigned char* median(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size) {
	std::array<int, 8> offset = getNeighborOffsets(w);
	int hist[255] = {0};
	for (int i = 3; i < size; i += 4) {
		for (int j = 0; j < 8; j += 1) {
			hist[inputBuf[i + offset[j]]] += 1;
		}
		int v = 0;
		int sum = 0;
		while (sum < 5) {
			sum += hist[v];
			v += 1;
		}
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = 255 - (v * 255 - 255);
		for (int j = 0; j < 8; j += 1) {
			hist[inputBuf[i + offset[j]]] = 0;
		}
	}
	return outputBuf;
}

#ifdef __cplusplus
}
#endif