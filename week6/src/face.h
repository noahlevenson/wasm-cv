#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// Class declarations
class Haarlike;
class WeakClassifier;
class PotentialWeakClassifier;
class StrongClassifier;

// Function prototypes
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE int getRectangleSum(std::vector<std::vector<int>>& integral, Wasmcv* project, int x, int y, int w, int h);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> generateFeatures(int s);
EMSCRIPTEN_KEEPALIVE int computeFeature(std::vector<std::vector<int>>& integral, Haarlike& haarlike, Wasmcv* project, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarA(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarB(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarC(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarD(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarE(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE bool comparePotentialWeakClassifiers(PotentialWeakClassifier a, PotentialWeakClassifier b);
EMSCRIPTEN_KEEPALIVE void trainCascade(int posCount, int negCount);

// Class for a Haar-like feature
class Haarlike {
	public:
		Haarlike(int s, int x, int y, int w, int h, int type, int f) {
			this->s = s; // Subwindow size
			this->x = x; // X offset
			this->y = y; // Y offset
			this->w = w; // Width
			this->h = h; // Height
			this->type = type; // 1 - 5 : A - E
			this->f = f; // The feature's computed value
		}
		Haarlike() {

		}
		int s;
		int x;
		int y;
		int w;
		int h;
		int type;
		int f;
};

// Class for a weak classifier
class WeakClassifier {
	public:
		WeakClassifier(Haarlike haarlike, int threshold, int polarity) {
			this->haarlike = haarlike;
			this->threshold = threshold;
			this->polarity = polarity;
		}
		WeakClassifier() {

		}
		Haarlike haarlike;
		int threshold;
		int polarity;
};

// Class for a potential weak classifier
// TODO - just use WeakClassifier for potentials, and delete them if they don't pass
class PotentialWeakClassifier {
	public:
		PotentialWeakClassifier(int f, bool positive, float w) {
			this->f = f;
			this->positive = positive;
			this->w = w;
		}
		PotentialWeakClassifier() {

		}
		int f;
		bool positive;
		float w;
};

// Class for a strong classifier
class StrongClassifier {
	public:
		StrongClassifier() {

		}
		std::vector<WeakClassifier> weakClassifiers;
		int threshold;
};

#ifdef __cplusplus
}
#endif