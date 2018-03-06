#ifdef __cplusplus
extern "C" {
#endif

// Class declarations
class NeighborhoodOffsets;
class BinaryStructuringElement2x2;
class BinaryStructuringElement3x3;
class BinaryStructuringElement5x5;
class Se;
class Wasmcv;
class Pixel;
class ImageBuffer;

// Function prototypes
EMSCRIPTEN_KEEPALIVE Wasmcv* init(int w, int h);
EMSCRIPTEN_KEEPALIVE bool isInImageBounds(Wasmcv* project, int offset);

#ifdef __cplusplus
}
#endif