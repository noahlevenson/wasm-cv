#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* median(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* medianRGBA(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* tmf(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* rank(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size, int r);

#ifdef __cplusplus
}
#endif