#ifdef __cplusplus
extern "C" {
#endif

// Threshold a grayscale image to binary
EMSCRIPTEN_KEEPALIVE unsigned char* binarize(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int t) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = 0;
		outputBuf[i + 1] = 0;
		outputBuf[i + 2] = 0;
		outputBuf[i + 3] = inputBuf[i + 3] > t ? 255 : 0;
	}
	return outputBuf;
}

// Median filter a grayscale image
EMSCRIPTEN_KEEPALIVE unsigned char* median(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	std::array<int, 9> o = project->offsets._3x3;
	int hist[256] = {0};
	for (int i = 3; i < project->size; i += 4) {
		for (int j = 0; j < 9; j += 1) {
			hist[inputBuf[i + o[j]]] += 1;
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
		outputBuf[i] = v - 1;
		for (int j = 0; j < 9; j += 1) {
			hist[inputBuf[i + o[j]]] = 0;
		}
	}
	return outputBuf;
}

// Rank order filter a grayscale image
EMSCRIPTEN_KEEPALIVE unsigned char* rank(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int r) {
	std::array<int, 9> o = project->offsets._3x3;
	int hist[256] = {0};
	for (int i = 3; i < project->size; i += 4) {
		for (int j = 0; j < 9; j += 1) {
			hist[inputBuf[i + o[j]]] += 1;
		}
		int v = 0;
		int sum = 0;
		while (sum < r) {
			sum += hist[v];
			v += 1;
		}
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = v - 1;
		for (int j = 0; j < 9; j += 1) {
			hist[inputBuf[i + o[j]]] = 0;
		}
	}
	return outputBuf;
}

// Truncated median filter a grayscale image
// TODO: Optimize this; question why it exists; question the math used to find min/max/median (i suspect some off-by-one errors)
EMSCRIPTEN_KEEPALIVE unsigned char* tmf(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	std::array<int, 9> o = project->offsets._3x3;
	int hist[256] = {0};
	for (int i = 3; i < project->size; i += 4) {
		// first make the big histogram representing the range of values found in the input pixel's neighbor pixels
		for (int j = 0; j < 9; j += 1) {
			hist[inputBuf[i + o[j]]] += 1;
		}
		// in the big histogram, find the median value
		int median = 0;
		int sum = 0;
		while (sum < 5) {
			sum += hist[median];
			median += 1;
		}
		median -= 1;
		// in the big histogram, find the minimum value
		int min = 255;
		sum = 9;
		while (sum > 0) {
			sum -= hist[min];
			min -= 1;
		}
		min += 1;
		// in the big histogram, find the maximum value
		int max = 0;
		sum = 0;
		while (sum < 9) {
			sum += hist[max];
			max += 1;
		}
		max -= 1;
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		// Get distance from median to truncate (based on which extreme of the distribution is closer to the median)
		int d = std::min(max - median, median - min);
		// Average values in the truncated range
		int val = 0;
		int acc = 0;
		int len = d * 2;
		int k = median - d;
		while (k < len) {
			val += hist[k];
			acc += k;
			k += 1;
		}
		// That's your output pixel value!
		outputBuf[i] = k - 1 / val;
		// Efficiently clear the histogram
		for (int j = 0; j < 9; j += 1) {
			hist[inputBuf[i + o[j]]] = 0;
		}
	}
	return outputBuf;
}

#ifdef __cplusplus
}
#endif