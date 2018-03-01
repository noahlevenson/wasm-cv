#ifdef __cplusplus
extern "C" {
#endif

// Threshold a grayscale image to binary
EMSCRIPTEN_KEEPALIVE unsigned char* threshold(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int t) {
	for (int i = 0; i < project->size; i += 4) {
		outputBuf[i] = 0;
		outputBuf[i + 1] = 0;
		outputBuf[i + 2] = 0;
		outputBuf[i + 3] = inputBuf[i + 3] > t ? 255 : 0;
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

// Otsu's method for thresholding
EMSCRIPTEN_KEEPALIVE unsigned char* otsu(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	// Build a histogram of values for every pixel in the input image
	unsigned char hist[256] = {0};
	for (int i = 3; i < project->size; i += 4) {
		hist[inputBuf[i]] += 1;
	}

	// Sum of foreground is the accumulated result of the weighted frequency of each luma level
	float sumOfForeground = 0;
	for (int t = 0; t < 256; t += 1) {
		sumOfForeground += t * hist[t];
	}

	float sumOfBackground = 0;
	int backgroundWeight = 0;
	int foregroundWeight = 0;

	float varMax = 0;
	int thresh = 0;

	int length = project->size / 4;

	for (int t = 0; t < 256; t += 1) {
		// This appears to be accumulating all the luma values distributed across the entire image?
		backgroundWeight += hist[t];

		// Foreground weight is the size of the image minus the total number of luma values distributed across the image?
		foregroundWeight = length - backgroundWeight;


		// Sum of background is the accumulated result of the weighted frequency of each luma level? Confusing!
		sumOfBackground += t * hist[t];


   		float mB = sumOfBackground / backgroundWeight;            			  // Mean Background
   		
   		float mF = (sumOfForeground - sumOfBackground) / foregroundWeight;    // Mean Foreground


   		if (backgroundWeight != 0 && backgroundWeight != 1) {

   			// This calculates the "between class" variance
			float varBetween = float(backgroundWeight) * float(foregroundWeight) * (mB - mF) * (mB - mF);

			// If a new maximum between class variance has been found, then set it as our new all-time-high
	   		if (varBetween > varMax) {
	      		varMax = varBetween;
	      		thresh = t;
   			}

   			std::cout << varBetween << std::endl;

   		}
		
	}
	//std::cout << thresh << std::endl;

	outputBuf = threshold(inputBuf, outputBuf, project, thresh);
	return outputBuf;

}

// Median filter a grayscale image using a 3x3 neighborhood sample
EMSCRIPTEN_KEEPALIVE unsigned char* median3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
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