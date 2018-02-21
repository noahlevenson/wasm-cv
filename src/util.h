#ifdef __cplusplus
extern "C" {
#endif

// Class declarations
class Pixel;
class ImageBuffer;

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