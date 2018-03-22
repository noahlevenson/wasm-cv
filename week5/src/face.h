#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE std::vector<int> makeIntegralImage(unsigned char inputBuf[], Wasmcv* project);

#ifdef __cplusplus
}
#endif