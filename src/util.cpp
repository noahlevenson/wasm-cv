#ifdef __cplusplus
extern "C" {
#endif

// Class for a collection of pixel neighborhood linear offsets
class NeighborhoodOffsets {
	public:
	NeighborhoodOffsets(int w) {
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

// Class for a 3x3 binary structuring element
class BinaryStructuringElement3x3 {
	public:
		BinaryStructuringElement3x3(std::array<unsigned char, 9> kernel) {
			this->w = 3;
			this->h = 3;
			this->size = 9;
			this->kernel = kernel;
			this->p0 = 4;
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
		int p0;
};

// Class for a 5x5 binary structuring element
class BinaryStructuringElement5x5 {
	public:
		BinaryStructuringElement5x5(std::array<unsigned char, 25> kernel) {
			this->w = 5;
			this->h = 5;
			this->size = 25;
			this->kernel = kernel;
			this->p0 = 12;
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
		int p0;
};

// Static class for a library of structuring elements (used for morphological erosion, dilation, opening and closing)
class Se {
	public:
		Se(void) {
			std::array<unsigned char, 9> _3x3iso = {255, 255, 255, 255, 255, 255, 255, 255, 255}; 
			this->_3x3iso = BinaryStructuringElement3x3(_3x3iso);
			std::array<unsigned char, 25> _5x5iso = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
			this->_5x5iso = BinaryStructuringElement5x5(_5x5iso);
			std::array<unsigned char, 25> _5x5disc = {0, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255 ,255, 255, 0};
			this->_5x5disc = BinaryStructuringElement5x5(_5x5disc);
			std::array<unsigned char, 25> _5x5ring = {0, 255, 255, 255, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 255, 255, 255, 0};
			this->_5x5ring = BinaryStructuringElement5x5(_5x5ring);
		}
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
		this->h = w;
		this->size = w * h * 4;
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
	NeighborhoodOffsets offsets;
	Se se;
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