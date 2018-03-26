#include <emscripten/emscripten.h>
#include <array>
#include <cmath>

#include "util.h"
#include "gray.h"

#ifdef __cplusplus
extern "C" {
#endif

// Threshold a grayscale image to binary (simple)
EMSCRIPTEN_KEEPALIVE unsigned char* threshold(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, int t) {
	unsigned char* outputBuf = pool->getNew();
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = 0;
		outputBuf[i + 1] = 0;
		outputBuf[i + 2] = 0;
		outputBuf[i + 3] = inputBuf[i + 3] < t ? 0 : 255;
	}
	return outputBuf;
}

// Simple adaptive thresholding using a 19x19 neighborhood sample, useful for binarizing print copy
EMSCRIPTEN_KEEPALIVE unsigned char* thresholdOCR(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	std::array<int, 361> o = project->offsets._19x19;
	const int minrange = 255 / 5; // This constant represents the minimum likely difference between foreground and background intensity
	for (int i = 3; i < project->size; i += 4) {
		int hist[361];
		int t;
		for (int j = 0; j < 361; j += 1) {
			hist[j] = inputBuf[i + o[j]];
		}
		std::sort(hist, hist + 361);
		int range = hist[360] - hist[0];
		if (range > minrange) {
			t = (hist[0] + hist[360]) / 2;
		} else {
			t = (hist[360] - minrange) / 2;
		}
		outputBuf[i] = inputBuf[i] > t ? 0 : 255;
	}
	return outputBuf;
}

// Otsu's method for automatic thresholding
// This method assumes a bimodal luma distribution and seeks to find the optimal threshold
// by separating luma values into two classes and finding the smallest between-class variance
// for every possible threshold
EMSCRIPTEN_KEEPALIVE unsigned char* otsu(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	// Bucket sort the luma values from our input image into a histogram
	unsigned char hist[256] = {0};
	for (int i = 3; i < project->size; i += 4) {
		hist[inputBuf[i]] += 1;
	}
	// Init some useful variables
	float vMax = 0;
	int thresh = 0;
	// For every possible threshold value, split the luma distribution
	// into two classes (foreground and background) and find the between-class variance
	for (int t = 0; t < 256; t += 1) {
		// Get weight of background class
		int bSum = 0, wbSum = 0;
		for (int i = 0; i < t; i += 1) {
			bSum += hist[i];
			wbSum += hist[i] * i;
		}
		float wB = float(bSum) / float(project->pixelArea);
		// Get weight of foreground class
		int fSum = 0, wfSum = 0;
		for (int i = t; i < 256; i += 1) {
			fSum += hist[i];
			wfSum += hist[i] * i;
		}
		float wF = float(fSum) / float(project->pixelArea);
		// Get mean of background and foreground class
		float bMean = float(wbSum) / float(bSum);
		float fMean = float(wfSum) / float(fSum);
		// Calculate between-class variance
		float m = (wB * bMean) + (wF * fMean);
		float v = (wB * std::pow(bMean - m, 2)) + (wF * std::pow(fMean - m, 2));
		// Select the maximum between class variance 
		if (v > vMax) {
			vMax = v;
			thresh = t;
		}
	}
	return threshold(inputBuf, pool, project, thresh);
}

// Median filter a grayscale image using a 3x3 neighborhood sample
EMSCRIPTEN_KEEPALIVE unsigned char* median3x3(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project) {
	unsigned char* outputBuf = pool->getNew();
	std::array<int, 9> o = project->offsets._3x3;
	unsigned char hist[9];
	for (int i = 3; i < project->size; i += 4) {
		for (int j = 0; j < 9; j += 1){
			hist[j] = inputBuf[i + o[j]];
		}
		// This is a sorting network - using it instead of std::sort
		// provides a speed boost of ~25ms/frame
		if (hist[0] > hist[1]) std::swap(hist[0], hist[1]);
		if (hist[3] > hist[4]) std::swap(hist[3], hist[4]);
		if (hist[6] > hist[7]) std::swap(hist[6], hist[7]);
		if (hist[1] > hist[2]) std::swap(hist[1], hist[2]);
		if (hist[4] > hist[5]) std::swap(hist[4], hist[5]);
		if (hist[7] > hist[8]) std::swap(hist[7], hist[8]);
		if (hist[0] > hist[1]) std::swap(hist[0], hist[1]);
		if (hist[3] > hist[4]) std::swap(hist[3], hist[4]);
		if (hist[6] > hist[7]) std::swap(hist[6], hist[7]);
		if (hist[2] > hist[5]) std::swap(hist[2], hist[5]);
		if (hist[1] > hist[4]) std::swap(hist[1], hist[4]);
		if (hist[3] > hist[6]) std::swap(hist[3], hist[6]);
		if (hist[4] > hist[7]) std::swap(hist[4], hist[7]);
		if (hist[2] > hist[5]) std::swap(hist[2], hist[5]);
		if (hist[1] > hist[4]) std::swap(hist[1], hist[4]);
		if (hist[2] > hist[6]) std::swap(hist[2], hist[6]);
		if (hist[4] > hist[6]) std::swap(hist[4], hist[6]);
		if (hist[2] > hist[4]) std::swap(hist[2], hist[4]);
		// std::sort(hist, hist + 9);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
	    outputBuf[i] = hist[4];
	}
	return outputBuf;
}

// Rank order filter a grayscale image using a 3x3 neighborhood sample
// TODO: Make this use a sorting network instead of std::sort
EMSCRIPTEN_KEEPALIVE unsigned char* rank3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int r = 5) {
	std::array<int, 9> o = project->offsets._3x3;
	for (int i = 3; i < project->size; i += 4) {
		unsigned char hist[9] = {0};
		for (int j = 0; j < 9; j += 1) {
			hist[j] = inputBuf[i + o[j]];
		}
		std::sort(hist, hist + 9);
		outputBuf[i - 3] = 0;
		outputBuf[i - 2] = 0;
		outputBuf[i - 1] = 0;
		outputBuf[i] = hist[r];
	}
	return outputBuf;
}

#ifdef __cplusplus
}
#endif