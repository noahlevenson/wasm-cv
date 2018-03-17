#include <emscripten/emscripten.h>
#include <array>

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
class BufferPool;
class Pixel;
class ImageBuffer;


// Function prototypes
EMSCRIPTEN_KEEPALIVE Wasmcv* init(int w, int h);
EMSCRIPTEN_KEEPALIVE bool isInImageBounds(Wasmcv* project, int offset);
EMSCRIPTEN_KEEPALIVE std::vector<int> offsetToVec2(int offset, Wasmcv* project);

// Class for a collection of pixel neighborhood 1D offsets
class NeighborhoodOffsets {
	public:
	NeighborhoodOffsets(int w) {
		this->_4n[0] = -w * 4;
		this->_4n[1] = -4;
		this->_4n[2] = 0;
		this->_4n[3] = 4;
		this->_4n[4] = w * 4; 
		this->_2x2[0] = 0;
		this->_2x2[1] = 4;
		this->_2x2[2] = w * 4;
		this->_2x2[3] = w * 4 + 4;
		for (int i = -1, idx = 0; i <= 1; i += 1) {
			for (int j = -1; j <= 1; j += 1, idx += 1) {
				this->_3x3[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -2, idx = 0; i <= 2; i += 1) {
			for (int j = -2; j <= 2; j += 1, idx += 1) {
				this->_5x5[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -3, idx = 0; i <= 3; i += 1) {
			for (int j = -3; j <= 3; j += 1, idx += 1) {
				this->_7x7[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -4, idx = 0; i <= 4; i += 1) {
			for (int j = -4; j <= 4; j += 1, idx += 1) {
				this->_9x9[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -5, idx = 0; i <= 5; i += 1) {
			for (int j = -5; j <= 5; j += 1, idx += 1) {
				this->_11x11[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -6, idx = 0; i <= 6; i += 1) {
			for (int j = -6; j <= 6; j += 1, idx += 1) {
				this->_13x13[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -7, idx = 0; i <= 7; i += 1) {
			for (int j = -7; j <= 7; j += 1, idx += 1) {
				this->_15x15[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -8, idx = 0; i <= 8; i += 1) {
			for (int j = -8; j <= 8; j += 1, idx += 1) {
				this->_17x17[idx] = w * i * 4 + j * 4;
			}
		}
		for (int i = -7, idx = 0; i <= 7; i += 1) {
			for (int j = -7; j <= 7; j += 1, idx += 1) {
				this->_19x19[idx] = w * i * 4 + j * 4;
			}
		}
	}
	NeighborhoodOffsets() {

	}
	std::array<int, 5> _4n;
	std::array<int, 4> _2x2;
	std::array<int, 9> _3x3;
	std::array<int, 25> _5x5;
	std::array<int, 49> _7x7;
	std::array<int, 81> _9x9;
	std::array<int, 121> _11x11;
	std::array<int, 169> _13x13;
	std::array<int, 225> _15x15;
	std::array<int, 289> _17x17;
	std::array<int, 361> _19x19;
};

// Class for a 2x2 binary structuring element
class BinaryStructuringElement2x2 {
	public:
		BinaryStructuringElement2x2(std::array<unsigned char, 4> kernel) {
			this->w = 2;
			this->h = 2;
			this->size = 4;
			this->kernel = kernel;
			this->origin = 0;
			this->positives = 0;
			for (int i = 0; i < 4; i += 1) {
				if (kernel[i] == 255) {
					this->positives += 1;
				}
			}
		}
		BinaryStructuringElement2x2() {

		}
		std::array<unsigned char, 4> kernel;
		int w;
		int h;
		int size;
		int positives;
		int origin;
};

// Class for a 3x3 binary structuring element
class BinaryStructuringElement3x3 {
	public:
		BinaryStructuringElement3x3(std::array<unsigned char, 9> kernel) {
			this->w = 3;
			this->h = 3;
			this->size = 9;
			this->kernel = kernel;
			this->origin = 4;
			this->positives = 0;
			for (int i = 0; i < 9; i += 1) {
				if (kernel[i] == 255) {
					this->positives += 1;
				}
			}
		}
		BinaryStructuringElement3x3() {

		}
		std::array<unsigned char, 9> kernel;
		int w;
		int h;
		int size;
		int positives;
		int origin;
};

// Class for a 5x5 binary structuring element
class BinaryStructuringElement5x5 {
	public:
		BinaryStructuringElement5x5(std::array<unsigned char, 25> kernel) {
			this->w = 5;
			this->h = 5;
			this->size = 25;
			this->kernel = kernel;
			this->origin = 12;
			this->positives = 0;
			for (int i = 0; i < 25; i += 1) {
				if (kernel[i] == 255) {
					this->positives += 1;
				}
			}
		}
		BinaryStructuringElement5x5() {

		}
		std::array<unsigned char, 25> kernel;
		int w;
		int h;
		int size;
		int positives;
		int origin;
};

// Static class for a library of structuring elements (used for morphological erosion, dilation, opening and closing)
class Se {
	public:
		Se() {
			std::array<unsigned char, 9> _3x3iso = {255, 255, 255, 255, 255, 255, 255, 255, 255}; 
			this->_3x3iso = BinaryStructuringElement3x3(_3x3iso);
			std::array<unsigned char, 25> _5x5iso = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
			this->_5x5iso = BinaryStructuringElement5x5(_5x5iso);
			std::array<unsigned char, 25> _5x5disc = {0, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255 ,255, 255, 0};
			this->_5x5disc = BinaryStructuringElement5x5(_5x5disc);
			std::array<unsigned char, 25> _5x5ring = {0, 255, 255, 255, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 255, 255, 255, 0};
			this->_5x5ring = BinaryStructuringElement5x5(_5x5ring);
			std::array<unsigned char, 4> _2x2eclr = {0, 0, 0, 255};
			this->_2x2eclr = BinaryStructuringElement2x2(_2x2eclr);
			std::array<unsigned char, 4> _2x2ecll = {0, 0, 255, 0};
			this->_2x2ecll = BinaryStructuringElement2x2(_2x2ecll);
			std::array<unsigned char, 4> _2x2ecul = {255, 0, 0, 0};
			this->_2x2ecul = BinaryStructuringElement2x2(_2x2ecul);
			std::array<unsigned char, 4> _2x2ecur = {0, 255, 0, 0};
			this->_2x2ecur = BinaryStructuringElement2x2(_2x2ecur);
			std::array<unsigned char, 4> _2x2icul = {255, 255, 255, 0};
			this->_2x2icul = BinaryStructuringElement2x2(_2x2icul);
			std::array<unsigned char, 4> _2x2icur = {255, 255, 0, 255};
			this->_2x2icur = BinaryStructuringElement2x2(_2x2icur);
			std::array<unsigned char, 4> _2x2icll = {255, 0, 255, 255};
			this->_2x2icll = BinaryStructuringElement2x2(_2x2icll);
			std::array<unsigned char, 4> _2x2iclr = {0, 255, 255, 255};
			this->_2x2iclr = BinaryStructuringElement2x2(_2x2iclr);
		}
		BinaryStructuringElement2x2 _2x2eclr;
		BinaryStructuringElement2x2 _2x2ecll;
		BinaryStructuringElement2x2 _2x2ecul;
		BinaryStructuringElement2x2 _2x2ecur;
		BinaryStructuringElement2x2 _2x2icul;
		BinaryStructuringElement2x2 _2x2icur;
		BinaryStructuringElement2x2 _2x2icll;
		BinaryStructuringElement2x2 _2x2iclr;
		BinaryStructuringElement3x3 _3x3iso;
		BinaryStructuringElement5x5 _5x5iso;
		BinaryStructuringElement5x5 _5x5disc;
		BinaryStructuringElement5x5 _5x5ring;
};

// Class for a wasm-cv project 
class Wasmcv {
	public:
		Wasmcv(int w, int h) {
		this->w = w;
		this->h = h;
		this->size = w * h * 4;
		this->centerX = w / 2;
		this->centerY = h / 2;
		NeighborhoodOffsets neighborhoodOffsets(w);
		this->offsets = neighborhoodOffsets;
		Se se;
		this->se = se;
		} 
		Wasmcv() {

		}
		int w;
		int h;
		int size;
		int centerX;
		int centerY;
		NeighborhoodOffsets offsets;
		Se se;
};

// Class for a buffer pool
class BufferPool {
	public:
		BufferPool(int w, int h) {
			this->size = w * h * 4;
			this->buffers[0] = new unsigned char[this->size];
			this->buffers[1] = new unsigned char[this->size];
			this->buffers[2] = new unsigned char[this->size];
			this->buffers[3] = new unsigned char[this->size];
			this->buffers[4] = new unsigned char[this->size];
			this->buffers[5] = new unsigned char[this->size];
			this->current = 0;
		}
		BufferPool() {

		}
		~BufferPool() {
			delete [] this->buffers[0];
			delete [] this->buffers[1];
			delete [] this->buffers[2];
			delete [] this->buffers[3];
			delete [] this->buffers[4];
			delete [] this->buffers[5];
		}
		unsigned char* getNew() {
			next();
			return this->buffers[current];
		}	
		unsigned char* getCurrent() {
			return this->buffers[current];
		}
		unsigned char* copyToNew(unsigned char inputBuf[]) {
			next();
			for (int i = 0; i < this->size; i += 1) {
				this->buffers[current][i] = inputBuf[i];
			}
			return this->buffers[current];
		}
	private:
		void next() {
			this->current = this->current == 5 ? 0 : this->current += 1;
		}
		int current;
		int size;
		std::array<unsigned char*, 6> buffers;
};

// Class for a HTML5 canvas pixel
// TODO: Actually use this?
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
// TODO: Actually use this?
class ImageBuffer {
	public:
		ImageBuffer(int w, int h) {
			this->w = w;
			this->h = h;
			this->size = w * h * 4;
			this->data = new unsigned char[w * h];
		}
		ImageBuffer() {

		}
		~ImageBuffer() {
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