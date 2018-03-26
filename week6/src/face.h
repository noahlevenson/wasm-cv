#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE std::vector<int> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE void computeHaarA(unsigned char inputBuf[], Wasmcv* project, int s);
EMSCRIPTEN_KEEPALIVE std::vector<std::vector<int>> getGeomHaarA(int sx, int sy, int rx, int ry, int w, int h);

#ifdef __cplusplus
}
#endif