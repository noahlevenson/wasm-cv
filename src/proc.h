#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
std::array<int, 9> getNeighborOffsets(int w);
std::array<int, 2> offsetToVec2(int offset, int w, int h);
int getSigma(unsigned char inputBuf[], int p0, int w);
bool isInImageBounds(int offset, int w, int h);
EMSCRIPTEN_KEEPALIVE unsigned char* level(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
EMSCRIPTEN_KEEPALIVE unsigned char* levelR(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char r);
EMSCRIPTEN_KEEPALIVE unsigned char* levelG(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char g);
EMSCRIPTEN_KEEPALIVE unsigned char* levelB(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char b);
EMSCRIPTEN_KEEPALIVE unsigned char* levelA(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, unsigned char a);
EMSCRIPTEN_KEEPALIVE unsigned char* invert(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftLeft(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftRight(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftUp(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* shiftDown(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int d);
EMSCRIPTEN_KEEPALIVE unsigned char* lighten(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int v);
EMSCRIPTEN_KEEPALIVE unsigned char* darken(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int v);
EMSCRIPTEN_KEEPALIVE unsigned char* toGrayscale(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* toBinary(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size, int t);
EMSCRIPTEN_KEEPALIVE unsigned char* dilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* kDilate(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* kErode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* erode(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* findEdges(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* removeSaltPepper(unsigned char inputBuf[], unsigned char outputBuf[], int w, int h, int size); 
EMSCRIPTEN_KEEPALIVE unsigned char* suppressNoise(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);
EMSCRIPTEN_KEEPALIVE unsigned char* suppressNoisePlus(unsigned char inputBuf[], unsigned char outputBuf[], int w, int size);

#ifdef __cplusplus
}
#endif