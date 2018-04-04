#include <emscripten/emscripten.h>
#include <vector>
#include <algorithm>
#include <iostream>

#include "util.h"
#include "face.h"
#include "rgba.h"

#ifdef __cplusplus
extern "C" {
#endif

Wasmcv* trainingProject = new Wasmcv(24, 24);
BufferPool* trainingBufferPool = new BufferPool(24, 24);

// Create an integral image from a grayscale image buffer
// An integral image at location (x, y) contains the sum of the pixels above and to the left of (x, y), inclusive
// Returns a 2D pixel-level intermediate representation
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project) {
	// 2D Data structure for our integral image
	std::vector<std::vector<int>> integral;
	integral.resize(project->w, std::vector<int>(project->h, 0));
	// Cache cumulative column values in a table
	std::vector<int> sumTable(project->size);
	// Loop through our input image 
	for (int i = 3; i < project->size; i += 4) {
		auto vec2 = offsetToVec2(i - 3, project);
		// Just for code comprehension
		int x = vec2[0];
		int y = vec2[1];
		// Value of our integral image at (x,y) should equal
		// the accumulated sum of the pixels in the column above it 
		// plus the accumulated sum of the pixels to the left
		int yP = y - 1 < 0 ? 0 : sumTable[i - project->w * 4];  // northern neighbor
		int xP = x - 1 < 0 ? 0 : integral[x - 1][y];  // left hand neighbor
		sumTable[i] = yP + inputBuf[i];
		integral[x][y] = xP + sumTable[i];	
	}
	return integral;
}

// Compute the sum of pixels in an integral image within a rectangle of arbitrary size 
// a +-----------------+ b
//   |                 |
//   |                 |
//   |                 |
//   |                 |
// d +-----------------+ c
EMSCRIPTEN_KEEPALIVE int getRectangleSum(std::vector<std::vector<int>>& integral, Wasmcv* project, int x, int y, int w, int h) {
	if (x != 0 && y != 0) {
		// CASE A: x != 0 && y != 0
		int a = integral[x - 1][y - 1];
		int b = integral[x + w][y - 1];
		int c = integral[x + w][y + h];
		int d = integral[x - 1][y + h];
		int sum = c + a - (b + d);
		return sum;
	} else if (x == 0 && y != 0) {
		// CASE B: x == 0
		int b = integral[x + w][y - 1];
		int c = integral[x + w][y + h];
		int sum = c - b;
		return sum;
	} else if (y == 0 && x != 0) {
		// CASE C: y == 0
		int c = integral[x + w][y + h];
		int d = integral[x - 1][y + h];
		int sum = c - d;
		return sum;
	} else {
		// CASE D: x == 0 && y == 0
		int c = integral[x + w][y + h];
		return c;
	}	
}

// Compute Haar-like feature type A (as described in the Wang paper) over all possible scales and
// positions for a given subwindow pixel and location
// Returns a std::vector of all recorded features
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarA(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	std::vector<Haarlike> features;
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type A feature comprised of rectangles of size (w, h) such that the feature 
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h <= s; h += 1) {
				for (int w = 1; w * 2 + j <= s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w - 1, h - 1);
					int blackSum = getRectangleSum(integral, project, j + sx + w, i + sy, w - 1, h - 1);
					int f = blackSum - whiteSum;
					Haarlike haarlike = Haarlike(s, j, i, w, h, 1, f);
					features.push_back(haarlike);
				}
			}
		}
	}
	return features;
}

// Compute Haar-like feature type B (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
// Returns a std::vector of all recorded features
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarB(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	std::vector<Haarlike> features;
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type B feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h <= s; h += 1) {
				for (int w = 1; w * 3 + j <= s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w - 1, h - 1) + 
						getRectangleSum(integral, project, j + sx + w * 2, i + sy, w - 1, h - 1);
					int blackSum = getRectangleSum(integral, project, j + sx + w, i + sy, w - 1, h - 1);
					int f = blackSum - whiteSum;
					Haarlike haarlike = Haarlike(s, j, i, w, h, 2, f);
					features.push_back(haarlike);
				}
			}
		}
	}
	return features;
}

// Compute Haar-like feature type C (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
// Returns a std::vector of all recorded features
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarC(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	std::vector<Haarlike> features;
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type C feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h * 2 <= s; h += 1) {
				for (int w = 1; j + w <= s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w - 1, h - 1);
					int blackSum = getRectangleSum(integral, project, j + sx, i + sy + h, w - 1, h - 1);
					int f = blackSum - whiteSum;
					Haarlike haarlike = Haarlike(s, j, i, w, h, 3, f);
					features.push_back(haarlike);
				}
			}
		}
	}
	return features;
}

// Compute Haar-like feature type D (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
// Returns a std::vector of all recorded features
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarD(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	std::vector<Haarlike> features;
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type D feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h * 3 <= s; h += 1) {
				for (int w = 1; j + w <= s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w - 1, h - 1) + 
						getRectangleSum(integral, project, j + sx, i + sy + h * 2, w - 1, h - 1);
					int blackSum = getRectangleSum(integral, project, j + sx, i + sy + h, w - 1, h - 1);
					int f = blackSum - whiteSum;
					Haarlike haarlike = Haarlike(s, j, i, w, h, 4, f);
					features.push_back(haarlike);
				}
			}
		}
	}
	return features;
}

// Compute Haar-like feature type E (as described in the Wang paper) over all possible scales and 
// positions for a given subwindow size and location
// Returns a std::vector of all recorded features
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarE(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy) {
	std::vector<Haarlike> features;
	// Loop through every possible pixel position in a subwindow of dimensions s X s
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			// Loop through every possible type E feature comprised of rectangles of size (w, h) such that the feature
			// remains constrained by the subwindow dimensions
			for (int h = 1; i + h * 2 <= s; h += 1) {
				for (int w = 1; j + w * 2 <= s; w += 1) {
					int whiteSum = getRectangleSum(integral, project, j + sx, i + sy, w - 1, h - 1) + 
						getRectangleSum(integral, project, j + sx + w, i + sy + h, w - 1, h - 1);
					int blackSum = getRectangleSum(integral, project, j + sx + w, i + sy, w - 1, h - 1) + 
						getRectangleSum(integral, project, j + sx, i + sy + h, w - 1, h - 1);
					int f = blackSum - whiteSum;
					Haarlike haarlike = Haarlike(s, j, i, w, h, 5, f);
					features.push_back(haarlike);
				}
			}
		}
	}
	return features;
}

// Derive an AdaBoost weak classifier from a single feature 
EMSCRIPTEN_KEEPALIVE bool weakClassifier(int feature, int polarity, int threshold) {
	if (polarity * feature > polarity * threshold) {
		return 1;
	} else {
		return 0;
	}
}

bool compare(Haarlike& a, Haarlike& b) {
	return a.f < b.f;
}

// Hook for train-cascade.js
EMSCRIPTEN_KEEPALIVE void trainCascade(int posCount, int negCount) {

	std::vector<int> test;

	// Loop through every positive image in the training dataset
	for (int i = 0; i < posCount; i += 1) {
		std::vector<Haarlike> positiveFeatures;
		int inputBufInt = EM_ASM_INT({
			const data = fs.readFileSync(positiveImages[$0]);
			img.src = data;
			ctx.drawImage(img, 0, 0, 24, 24);
			const inputImageData = ctx.getImageData(0, 0, 24, 24);
			const inputBuf = Module._malloc(inputImageData.data.length);
			Module.HEAPU8.set(inputImageData.data, inputBuf);
			return inputBuf;
		}, i);
		unsigned char* inputBuf = (unsigned char*)inputBufInt;
		unsigned char* grayscaled = toGrayscale(inputBuf, trainingBufferPool, trainingProject);
		auto integral = makeIntegralImage(trainingBufferPool->getCurrent(), trainingProject);

		std::cout << "Computing all features for positive example " << i + 1 << "/" << posCount << std::endl;

		auto featuresA = computeHaarA(integral, trainingProject, 24, 0, 0);
		auto featuresB = computeHaarB(integral, trainingProject, 24, 0, 0);
		auto featuresC = computeHaarC(integral, trainingProject, 24, 0, 0);
		auto featuresD = computeHaarD(integral, trainingProject, 24, 0, 0);
		auto featuresE = computeHaarE(integral, trainingProject, 24, 0, 0);

		positiveFeatures.insert(positiveFeatures.end(), featuresA.begin(), featuresA.end());
		positiveFeatures.insert(positiveFeatures.end(), featuresB.begin(), featuresB.end());
		positiveFeatures.insert(positiveFeatures.end(), featuresC.begin(), featuresC.end());
		positiveFeatures.insert(positiveFeatures.end(), featuresD.begin(), featuresD.end());
		positiveFeatures.insert(positiveFeatures.end(), featuresE.begin(), featuresE.end());

		for (int j = 0; j < 160000; j += 1) {
			test.push_back(179);
		}
		

		EM_ASM({
			Module._free($0); 
		}, inputBuf);

	}

	std::cout << test.size() << std::endl;

	// std::cout << "Sorting features...\n";
	// std::sort(positiveFeatures.begin(), positiveFeatures.end(), compare);
	// std::cout << "--> Smallest feature: " << positiveFeatures.front().f << std::endl;
	// std::cout << "--> Biggest feature: " << positiveFeatures.back().f << std::endl;

	// std::cout << "Total features: " << positiveFeatures.size() << std::endl;

	// Loop through every negative image in the training dataset
	// for (int i = 0; i < negCount; i += 1) {

	// 	std::vector<Haarlike> negativeFeatures;

	// 	int inputBufInt = EM_ASM_INT({
	// 		const data = fs.readFileSync(negativeImages[$0]);
	// 		img.src = data;
	// 		ctx.drawImage(img, 0, 0, 24, 24, 0, 0, 24, 24);
	// 		const inputImageData = ctx.getImageData(0, 0, 24, 24);
	// 		const inputBuf = Module._malloc(inputImageData.data.length);
	// 		Module.HEAPU8.set(inputImageData.data, inputBuf);
	// 		return inputBuf;
	// 	}, i);
	// 	unsigned char* inputBuf = (unsigned char*)inputBufInt;
	// 	unsigned char* grayscaled = toGrayscale(inputBuf, trainingBufferPool, trainingProject);
	// 	auto integral = makeIntegralImage(trainingBufferPool->getCurrent(), trainingProject);

	// 	std::cout << "Computing all features for negative example " << i + 1 << "/" << negCount << std::endl;

	// 	auto featuresA = computeHaarA(integral, trainingProject, 24, 0, 0);
	// 	auto featuresB = computeHaarB(integral, trainingProject, 24, 0, 0);
	// 	auto featuresC = computeHaarC(integral, trainingProject, 24, 0, 0);
	// 	auto featuresD = computeHaarD(integral, trainingProject, 24, 0, 0);
	// 	auto featuresE = computeHaarE(integral, trainingProject, 24, 0, 0);

	// 	negativeFeatures.insert(negativeFeatures.end(), featuresA.begin(), featuresA.end());
	// 	negativeFeatures.insert(negativeFeatures.end(), featuresB.begin(), featuresB.end());
	// 	negativeFeatures.insert(negativeFeatures.end(), featuresC.begin(), featuresC.end());
	// 	negativeFeatures.insert(negativeFeatures.end(), featuresD.begin(), featuresD.end());
	// 	negativeFeatures.insert(negativeFeatures.end(), featuresE.begin(), featuresE.end());

	// 	std::sort(negativeFeatures.begin(), negativeFeatures.end(), compare);

	// 	std::cout << "Total features: " << negativeFeatures.size() << std::endl;

	// 	std::cout << "Biggest negative feature: " << negativeFeatures.back().f << std::endl;

	// 	std::cout << "type: " << negativeFeatures.back().type << std::endl;
	// 	std::cout << "width: " << negativeFeatures.back().w << std::endl;
	// 	std::cout << "height: " << negativeFeatures.back().h << std::endl;
	// 	std::cout << "x offset: " << negativeFeatures.back().x << std::endl;
	// 	std::cout << "y offset: " << negativeFeatures.back().y << std::endl;


	// 	std::cout << "Smallest negative feature: " << negativeFeatures.front().f << std::endl;

	// 	std::cout << "type: " << negativeFeatures.front().type << std::endl;
	// 	std::cout << "width: " << negativeFeatures.front().w << std::endl;
	// 	std::cout << "height: " << negativeFeatures.front().h << std::endl;
	// 	std::cout << "x offset: " << negativeFeatures.front().x << std::endl;
	// 	std::cout << "y offset: " << negativeFeatures.front().y << std::endl;

	// 	EM_ASM({
	// 		Module._free($0); 
	// 	}, inputBuf);
	// }
}

#ifdef __cplusplus
}
#endif