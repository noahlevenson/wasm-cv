#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* binarize(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int t);
EMSCRIPTEN_KEEPALIVE unsigned char* median(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);
EMSCRIPTEN_KEEPALIVE unsigned char* rank(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project, int r);
EMSCRIPTEN_KEEPALIVE unsigned char* tmf(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project);

#ifdef __cplusplus
}
#endif