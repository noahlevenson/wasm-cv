#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
EMSCRIPTEN_KEEPALIVE unsigned char* toBinary(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int t);
EMSCRIPTEN_KEEPALIVE unsigned char* median(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* rank(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size, int r);
EMSCRIPTEN_KEEPALIVE unsigned char* tmf(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);

#ifdef __cplusplus
}
#endif