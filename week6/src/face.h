#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// Class declarations
class Haarlike;

// Function prototypes
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE int getRectangleSum(std::vector<std::vector<int>>& integral, Wasmcv* project, int x, int y, int w, int h);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarA(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarB(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarC(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarD(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE std::vector<Haarlike> computeHaarE(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE bool weakClassifier(int feature, int polarity, int threshold);

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

#ifdef __cplusplus
}
#endif