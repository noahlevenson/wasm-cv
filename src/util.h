#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
std::array<int, 9> getNeighborOffsets(int w);
std::array<int, 2> offsetToVec2(int offset, int w, int h);

// Class declarations
class Pixel;
class ImageBuffer;

#ifdef __cplusplus
}
#endif