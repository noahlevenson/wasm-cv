#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
int sumNeighbors(unsigned char inputBuf[], int p0, int w);
unsigned char* conv3(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size, int k[], int norm);
EMSCRIPTEN_KEEPALIVE unsigned char* dilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* erode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* kDilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* kErode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* findEdges(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* deSaltPepper(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size); 
EMSCRIPTEN_KEEPALIVE unsigned char* boxBlur(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* gaussianApprox(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);

#ifdef __cplusplus
}
#endif