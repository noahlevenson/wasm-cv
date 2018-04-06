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

// Generate a complete ~160,000 feature set of Haar-like features (Type A, B, C, D, E as described in the Wang paper) 
// over all scales and positions for a given subwindow size
// This does not compute features and will create features with a ->f property of 0
// returns a std::vector of Haarlike objects
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> generateFeatures(int s) {
	std::vector<Haarlike> features;
	// Type A
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			for (int h = 1; i + h <= s; h += 1) {
				for (int w = 1; w * 2 + j <= s; w += 1) {
					Haarlike haarlike = Haarlike(s, j, i, w, h, 1, 0);
					features.push_back(haarlike);
				}
			}
		}
	}
	// Type B
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			for (int h = 1; i + h <= s; h += 1) {
				for (int w = 1; w * 3 + j <= s; w += 1) {
					Haarlike haarlike = Haarlike(s, j, i, w, h, 2, 0);
					features.push_back(haarlike);
				}
			}
		}
	}
	// Type C
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			for (int h = 1; i + h * 2 <= s; h += 1) {
				for (int w = 1; j + w <= s; w += 1) {
					Haarlike haarlike = Haarlike(s, j, i, w, h, 3, 0);
					features.push_back(haarlike);
				}
			}
		}
	}
	// Type D
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			for (int h = 1; i + h * 3 <= s; h += 1) {
				for (int w = 1; j + w <= s; w += 1) {
					Haarlike haarlike = Haarlike(s, j, i, w, h, 4, 0);
					features.push_back(haarlike);
				}
			}
		}
	}
	// Type E
	for (int i = 0; i < s; i += 1) {
		for (int j = 0; j < s; j += 1) {
			for (int h = 1; i + h * 2 <= s; h += 1) {
				for (int w = 1; j + w * 2 <= s; w += 1) {
					Haarlike haarlike = Haarlike(s, j, i, w, h, 5, 0);
					features.push_back(haarlike);
				}
			}
		}
	}
	return features;
}

// Compute the feature value of one Haar-like feature over an integral image
EMSCRIPTEN_KEEPALIVE int computeFeature(std::vector<std::vector<int>>& integral, Haarlike& haarlike, Wasmcv* project, int sx, int sy) {
	if (haarlike.type == 1) {
		// Type A
		int whiteSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1);
		int blackSum = getRectangleSum(integral, project, haarlike.x + sx + haarlike.w, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1);
		int f = blackSum - whiteSum;
		return f;
	} else if (haarlike.type == 2) {
		// Type B
		int whiteSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1) + 
			getRectangleSum(integral, project, haarlike.x + sx + haarlike.w * 2, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1);
		int blackSum = getRectangleSum(integral, project, haarlike.x + sx + haarlike.w, haarlike.y + sy, haarlike.w - 1, haarlike.y - 1);
		int f = blackSum - whiteSum;
		return f;
	} else if (haarlike.type == 3) {
		// Type C
		int whiteSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1);
		int blackSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy + haarlike.h, haarlike.w - 1, haarlike.h - 1);
		int f = blackSum - whiteSum;
		return f;
	} else if (haarlike.type == 4) {
		// Type D
		int whiteSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1) + 
			getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy + haarlike.h * 2, haarlike.w - 1, haarlike.h - 1);
		int blackSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy + haarlike.h, haarlike.w - 1, haarlike.h - 1);
		int f = blackSum - whiteSum;
		return f;
	} else {
		// Type E
		int whiteSum = getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1) + 
			getRectangleSum(integral, project, haarlike.x + sx + haarlike.w, haarlike.y + sy + haarlike.h, haarlike.w - 1, haarlike.h - 1);
	    int blackSum = getRectangleSum(integral, project, haarlike.x + sx + haarlike.w, haarlike.y + sy, haarlike.w - 1, haarlike.h - 1) + 
			getRectangleSum(integral, project, haarlike.x + sx, haarlike.y + sy + haarlike.h, haarlike.w - 1, haarlike.h - 1);
	    int f = blackSum - whiteSum;
	    return f;
	}
}


// Compute Haar-like feature type A (as described in the Wang paper) over all possible scales and
// positions for a given subwindow size and location
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

// Classify a training image using a weak classifier
EMSCRIPTEN_KEEPALIVE bool classify(int featureValue, int threshold, int polarity) {
	if (featureValue * polarity > threshold * polarity) {
		return true;
	} else {
		return false;
	}
}

// Compare potential weak classifiers
EMSCRIPTEN_KEEPALIVE bool comparePotentialWeakClassifiers(PotentialWeakClassifier a, PotentialWeakClassifier b) {
	return a.f < b.f;
}

// Hook for train-cascade.js
// TODO: Break all this stuff out into appropriate classes and functions
EMSCRIPTEN_KEEPALIVE void trainCascade(int posCount, int negCount) {

	// Generate geometry for a complete set of 162,336 Haar-like features
	std::cout << "Generating set of 162,336 Haar-like features\n";
	auto featureSet = generateFeatures(24);

	// Data structure for our best weak classifiers
	std::vector<WeakClassifier> bestWeakClassifiers;

	// Init weights
	std::vector<float> posWeights(posCount);
	std::vector<float> negWeights(negCount);
	std::fill(posWeights.begin(), posWeights.end(), float(1) / (float(2) * float(posCount)));
	std::fill(negWeights.begin(), negWeights.end(), float(1) / (float(2) * float(negCount)));

	// Do for each of the 162,336 features
	for (int i = 0; i < featureSet.size(); i += 1) {

		// Data structure for computed feature values
		// Contains one feature's response across all training images
		std::vector<PotentialWeakClassifier> potentialWeakClassifiers;

		// Data structures for features computed over pos and neg example images
		// this duplicates potentialWeakClassifiers but remains unsorted?
		// Feels bad and broken
		std::vector<int> posFeatures;
		std::vector<int> negFeatures;

		// Normalize the weights!
		// TODO: We need a utility function to sum things up
		float wSum = 0;
		for (int j = 0; j < posCount; j += 1) wSum += posWeights[j];
		for (int j = 0; j < negCount; j += 1) wSum += negWeights[j];
		
		for (int j = 0; j < posCount; j += 1) {
			posWeights[j] /= wSum;
		}
		for (int j = 0; j < negCount; j += 1) {
			negWeights[j] /= wSum;
		}


		// Loop through every positive image in the training dataset
		for (int j = 0; j < posCount; j += 1) {
			int inputBufInt = EM_ASM_INT({
				const data = fs.readFileSync(positiveImages[$0]);
				img.src = data;
				ctx.drawImage(img, 0, 0, 24, 24);
				const inputImageData = ctx.getImageData(0, 0, 24, 24);
				const inputBuf = Module._malloc(inputImageData.data.length);
				Module.HEAPU8.set(inputImageData.data, inputBuf);
				return inputBuf;
			}, j);
			unsigned char* inputBuf = (unsigned char*)inputBufInt;
			unsigned char* grayscaled = toGrayscale(inputBuf, trainingBufferPool, trainingProject);
			auto integral = makeIntegralImage(trainingBufferPool->getCurrent(), trainingProject);

			int f = computeFeature(integral, featureSet[i], trainingProject, 0, 0);

			// std::cout << "Feature " << i << "/162336 over positive training example " << i << ": " << f << std::endl;

			potentialWeakClassifiers.push_back(PotentialWeakClassifier(f, true, posWeights[j]));
			posFeatures.push_back(f);
			
			EM_ASM({
				Module._free($0); 
			}, inputBuf);

		}

		//Loop through every negative image in the training dataset
		for (int j = 0; j < negCount; j += 1) {
			int inputBufInt = EM_ASM_INT({
				const data = fs.readFileSync(negativeImages[$0]);
				img.src = data;
				ctx.drawImage(img, 0, 0, 24, 24, 0, 0, 24, 24); // Note that we draw a 24 pixel subsection from the 0,0 corner
				const inputImageData = ctx.getImageData(0, 0, 24, 24);
				const inputBuf = Module._malloc(inputImageData.data.length);
				Module.HEAPU8.set(inputImageData.data, inputBuf);
				return inputBuf;
			}, j);
			unsigned char* inputBuf = (unsigned char*)inputBufInt;
			unsigned char* grayscaled = toGrayscale(inputBuf, trainingBufferPool, trainingProject);
			auto integral = makeIntegralImage(trainingBufferPool->getCurrent(), trainingProject);

			int f = computeFeature(integral, featureSet[i], trainingProject, 0, 0);

			// std::cout << "Feature " << p << "/162336 over negative training example " << i << ": " << f << std::endl;

			potentialWeakClassifiers.push_back(PotentialWeakClassifier(f, false, negWeights[j]));
			negFeatures.push_back(f);

			EM_ASM({
				Module._free($0); 
			}, inputBuf);
		}

		// Now we've computed one feature across all training images so we need to find its optimal threshold!
		// First sort the features
		std::sort(potentialWeakClassifiers.begin(), potentialWeakClassifiers.end(), comparePotentialWeakClassifiers);
		// Data structures
		std::vector<float> sumPosWeights(potentialWeakClassifiers.size(), 0);
		std::vector<float> sumNegWeights(potentialWeakClassifiers.size(), 0);
		std::vector<float> sumPosWeightsBelow(potentialWeakClassifiers.size(), 0);
		std::vector<float> sumNegWeightsBelow(potentialWeakClassifiers.size(), 0);

		std::cout << "\n\nTraining...\n";
		std::cout << "Finding optimal threshold for feature " << i << "/" << featureSet.size() << " across " << posCount + negCount << " example images\n";

		// Do for the value computed over each of the training images
		float negErr;
		float posErr;
		float minErr = 1;
		int threshold;
		int polarity;
		// TODO:
		// This feels like it could be broken...?
		for (int j = 0; j < potentialWeakClassifiers.size(); j += 1) {
			for (int k = 0; k < posCount; k += 1) {
				sumPosWeights[j] += posWeights[k]; // Isn't this a constant #???
			}
			for (int k = 0; k < j; k += 1) {
				if (potentialWeakClassifiers[k].positive) {
					sumPosWeightsBelow[j] += potentialWeakClassifiers[k].w;
				}
			}
			for (int k = 0; k < negCount; k += 1) {
				sumNegWeights[j] += negWeights[k]; // Isn't this a constant #???
			}
			for (int k = 0; k < j; k += 1) {
				if (!potentialWeakClassifiers[k].positive) {
					sumNegWeightsBelow[j] += potentialWeakClassifiers[k].w; 
				}
			}
			negErr = sumPosWeightsBelow[j] + (sumNegWeights[j] - sumNegWeightsBelow[j]); // minimum error of all examples below current example negative
			posErr = sumNegWeightsBelow[j] + (sumPosWeights[j] - sumPosWeightsBelow[j]); // minimum error of all examples above current example positive
			if (posErr < negErr) {
				if (posErr < minErr) {
					minErr = posErr;
					threshold = potentialWeakClassifiers[j].f;
					polarity = -1;
				}
			} else {
				if (negErr < minErr) {
					minErr = negErr;
					threshold = potentialWeakClassifiers[j].f;
					polarity = 1;
				}
			}
		}
		// Now we have selected the best weak classifier for this feature! So we can push it
		// into a data structure with parameters: featureSet[i], threshold, polarity
		std::cout << "Best weak classifier for feature " << i << ": " << "threshold " << threshold << ", polarity " << polarity << std::endl; 
		bestWeakClassifiers.push_back(WeakClassifier(featureSet[i], threshold, polarity));

		// Now it's time to update the weights!
		float beta = minErr / (float(1) - minErr);
		for (int j = 0; j < posCount; j += 1) {
			if (classify(posFeatures[j], threshold, polarity) == 1) {
				// std::cout << "Classified positive!\n";
				posWeights[j] *= beta;
			}
		}
		for (int j = 0; j < negCount; j += 1) {
			if (classify(negFeatures[j], threshold, polarity) == 0) {
				// std::cout << "Classified negative!\n";
				negWeights[j] *= beta;
			}
		}
	}

}

#ifdef __cplusplus
}
#endif