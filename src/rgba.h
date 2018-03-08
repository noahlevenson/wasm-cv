#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* level(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
EMSCRIPTEN_KEEPALIVE unsigned char* levelR(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char r);
EMSCRIPTEN_KEEPALIVE unsigned char* levelG(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char g);
EMSCRIPTEN_KEEPALIVE unsigned char* levelB(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char b);
EMSCRIPTEN_KEEPALIVE unsigned char* levelA(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, unsigned char a);
EMSCRIPTEN_KEEPALIVE unsigned char* invert(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftLeft(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftRight(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftUp(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftDown(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* lighten(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int v);
EMSCRIPTEN_KEEPALIVE unsigned char* darken(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int v);
EMSCRIPTEN_KEEPALIVE unsigned char* contrast(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int g, int b);
EMSCRIPTEN_KEEPALIVE unsigned char* toGrayscale(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* medianRGBA(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);

#ifdef __cplusplus
}
#endif