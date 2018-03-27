#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE int getRectangleSum(std::vector<std::vector<int>>& integral, Wasmcv* project, int x, int y, int w, int h);
EMSCRIPTEN_KEEPALIVE void computeHaarA(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE void computeHaarB(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE void computeHaarC(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE void computeHaarD(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE void computeHaarE(std::vector<std::vector<int>>& integral, Wasmcv* project, int s, int sx, int sy);
EMSCRIPTEN_KEEPALIVE bool weakClassifier(int feature, int polarity, int threshold);
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> getGeomHaarA(int sx, int sy, int rx, int ry, int w, int h);

#ifdef __cplusplus
}
#endif