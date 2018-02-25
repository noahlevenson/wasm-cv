#ifdef __cplusplus
extern "C" {
#endif

// Return neighborhood pixel offsets as an array
std::array<int, 9> getNeighborOffsets(int w) {
	int p0 = -w * 4 - 4;
	int p1 = -w * 4;
	int p2 = -w * 4 + 4;
	int p3 = -4;
	int p4 = 0;
	int p5 = 4;
	int p6 = w * 4 - 4;
	int p7 = w * 4;
	int p8 = w * 4 + 4;
	std::array<int, 9> offsets = {p0, p1, p2, p3, p4, p5, p6, p7, p8};
	return offsets;
}

// Convert an image buffer offset to x, y coords
std::array<int, 2> offsetToVec2(int offset, int w, int h) {
	int x = (offset / 4) % w;
	int y = (offset - (offset % w)) / (w * 4);
	std::array<int, 2> vec2 = {x, y};
	return vec2;
}

// Class for a HTML5 canvas pixel
class Pixel {
	public:
		Pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		};
		Pixel() {

		}
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
};

// Class for an image buffer 
class ImageBuffer {
	public:
		ImageBuffer(int w, int h) {
			this->w = w;
			this->h = h;
			this->size = w * h;
			this->data = new unsigned char[w * h];
		}
		ImageBuffer() {

		}
		void destroy() {
			delete [] this->data;
		}
		int w;
		int h;
		int size;
		unsigned char* data;
};

#ifdef __cplusplus
}
#endif