#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* threshold(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project, int t = 127);
EMSCRIPTEN_KEEPALIVE unsigned char* thresholdOCR(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* otsu(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* median3x3(unsigned char inputBuf[], BufferPool* pool, Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* rank3x3(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int r);

#ifdef __cplusplus
}
#endif