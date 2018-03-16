#include <iostream>
#include <algorithm>
#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* Remember to enable the C++17 std library when you compile!
* 
* g++ edgedetect.cpp -std=c++17 -o e
*
* emcc edgedetect.cpp -s WASM=1 -O3 -std=c++1z -o index.js
*
* there seems to be a bug or "feaure" that makes the ccall exported method unavailable to the js glue code...
* the below linker flags make sure that Module.ccall() is accessible in the browser!
*
* emcc edgedetect.cpp -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s WASM=1 -O3 -std=c++1z -o ../env/wasm-cv.js
*
* source ./emsdk_env.sh --build=Release
*
*/

// Do class declarations need to go inside the #ifdefs?
class Pixel;
class Point;

// Do function prototypes need to go inside the #ifdefs? 
unsigned char clamp(int n);
Point offsetToPoint(int offset, int width);
int pointToOffset(int x, int y, int width);
bool checkImageBounds(int x, int y, int w, int h);
Pixel getPixel(unsigned char framebuffer[], int x, int y, int width);
unsigned char* edgeDetect(unsigned char framebuffer[]);
unsigned char* boxBlur3x3(unsigned char framebuffer[]);
unsigned char* boxBlurAndEdgeDetect(unsigned char framebuffer[]);
unsigned char* sobelFeldman(unsigned char framebuffer[]);




// class for a pixel object as represented by a canvas imagedata object: r, g, b, a
class Pixel {
	public:
		Pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}
		Pixel() {

		}
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
};

//class for a point, or a 2D vector -- a pair of x/y coordinates
class Point {
	public:
		Point(int x, int y) {
			this->x = x;
			this->y = y;
		}
		Point () {

		}
		int x;
		int y;
};


// very slow and bad way to clamp a signed integer
// to unsigned char range - this is just here as a fallback
// for when the compiler can't usee c++17 headers
// which include std::clamp
unsigned char clamp(int n) {
	if (n > 255) n = 255;
	if (n < 0) n = 0;
	return (unsigned char)n;
}


// convert a memory offset to a 2d vector
Point offsetToPoint(int offset, int width) {
	int x = (offset / 4) % width;
	int y = (offset - (offset % width)) / (width * 4);
	Point point(x, y);
	return point;
}

// convert a 2d vector to a memory offset
int pointToOffset(int x, int y, int width) {
	return y * 4 * width  + (x * 4);
}

// this is a very slow and inefficent way to check
// if a specified 2d vector is within the boundaries
// of an image of given width and height
bool checkImageBounds(int x, int y, int w, int h) {
	if (x < w && x >= 0 && y < h && y >= 0) {
		return true;
	}
	else {
		return false;
	}
}

// this is a really, really slow operation
// we should probably figure out how to do this bitwise
// or just way faster
Point clampToImageEdges(int x, int y, int w, int h) {
	if (x >= w) x = w - 1;
	if (x < 0) x = 0;
	if (y >= h) y = h - 1;
	Point point(x, y);
	return point;
}

// is this performing a copy operation on the framebuffer array of unsigned chars?
// am i really supposed to do this with a pointer instead ?
// TODO: RESEARCH IT
Pixel getPixel(unsigned char framebuffer[], int x, int y, int width) {
	int offset = pointToOffset(x, y, width);
	Pixel p(framebuffer[offset], framebuffer[offset + 1], framebuffer[offset + 2], framebuffer[offset + 3]);
	return p;
}




/*
* If I understand my own code correctly, this C++ version of edgeDetect() takes one argument:
* - an source array of unsigned chars, which is javascript typedarray unsigned 8 bit integer type, aka Uint8Array 
* 
* this is passed by value, which is NOT GOOD... I think we're wasting a ton of cycles on a memory copy
* operation, and we should figure out how to pass it simply as pointers to the heap...
*
* anyway, it creates a destination buffer on the heap, and then returns a pointer to that buffer when it terminates
*
* so the return is an offset to a memory location on the heap.  if you access the HEAPU8 space
* on the Module in the javascript glue code using the offset returned here, you have accessed offset zero
* of the output image bitmap
*
* in the future, i might experiment with possibly trying to pass this function a pointer directly to a javascript
* imagedata bitmap, so that i don't have to allocate memory here in this function, then copy it to
* an imagedata object on the javascript side... it would be dope if I could just write directly to the imagedata
* object from within here!
*/
EMSCRIPTEN_KEEPALIVE unsigned char* edgeDetect(unsigned char framebuffer[]) {
	int neighborOffsets[9][2] = {{-1, 1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
	int convolutionMatrix[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};

	// here we create an empty buffer to put the output pixels into...
	// we'l return a pointer to this buffer in the heap for the js to pick up
	// note that we have to create it on the heap rather than the stack
	// using the NEW keyword...
	unsigned char* destinationBuffer = new unsigned char[1228800];

	// this here is the main loop where we iterate through the source image buffer
	// remember to change the i < 1000000 to i < length of source image buffer
	// we need to figure out a way to transfer that info from JS to this C++ function
	for (int offset = 0; offset < 1228800; offset += 4) {

		Point currentPoint = offsetToPoint(offset, 640);

		// note that this is handled differently from the JS version of this function...
		// here, we loop through our neighbor pixels, weight each RGB value by multiplying
		// it by our kernel, and then just accumulate those weighted values in the integers r,g,b
		// below... 

		// in the JS version, we create an array of weighted neighbor pixels, then push
		// weighted pixels values into it, then use a reduce function to accumulate
		// their values.  I think that's really not necessary.

		int r = 0;
		int g = 0;
		int b = 0;

		// loop through all 9 of our kernel pixels
		for (int i = 0; i < 9; i += 1) {
			int neighborX = currentPoint.x + neighborOffsets[i][0];
			int neighborY = currentPoint.y + neighborOffsets[i][1];
			// if the given neighbor pixel is in bounds, let's get its values, multiply them by 
			// their weight (as provided by the kernel), then accumulate the values
			if (checkImageBounds(neighborX, neighborY, 640, 480)) {
				
				Pixel neighbor = getPixel(framebuffer, neighborX, neighborY, 640);


				r += int(neighbor.r) * convolutionMatrix[i];
				g += int(neighbor.g) * convolutionMatrix[i];
				b += int(neighbor.b) * convolutionMatrix[i];

				
			}
			// if the neighbor pixel is out of image bounds, we'll do "EXTEND" edge mode
			// handling - we clamp the out of bounds point to the image edge and then
			// get our neighbor pixel, weight it and accumulate it
			else {
				Point clamped = clampToImageEdges(neighborX, neighborY, 640, 480);

				Pixel neighbor = getPixel(framebuffer, clamped.x, clamped.y, 640);

				r += int(neighbor.r) * convolutionMatrix[i];
				g += int(neighbor.g) * convolutionMatrix[i];
				b += int(neighbor.b) * convolutionMatrix[i];
			}

			


			//std::cout << offset << std::endl;
			

			//std::cout << "Output pixel, R: " << int(output.r) << " G: " << int(output.g) << " B: " << int(output.b) << " A: " << int(output.a) << std::endl;


		}


		// create an output pixel by clamping the weighted and accumulated RGB values to unsigned char range
			// note that we use framebuffer[offset + 3] for the alpha value, which just makes the output pixel alpha = the input pixel alpha

			// *** IMPORTANT!!! ***
			// certain compilers may not be up to speed with c++17, which is where we get the std::clamp function...
			// in that case, fall back to the below statement, which uses the slow clamp function i wrote...
			//Pixel output(clamp(r), clamp(g), clamp(b), framebuffer[offset + 3]);
			Pixel output(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), framebuffer[offset + 3]);

			destinationBuffer[offset] = output.r;
			destinationBuffer[offset + 1] = output.g;
			destinationBuffer[offset + 2] = output.b;
			destinationBuffer[offset + 3] = output.a;
 

	}
	
	//std::cout << "Done!\n";

	return destinationBuffer;
}





EMSCRIPTEN_KEEPALIVE unsigned char* boxBlur3x3(unsigned char framebuffer[]) {
	int neighborOffsets[9][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
	int convolutionMatrix[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

	unsigned char* destinationBuffer = new unsigned char[1228800];

	for (int offset = 0; offset < 1228800; offset += 4) {

		Point currentPoint = offsetToPoint(offset, 640);

		int r = 0;
		int g = 0;
		int b = 0;

		for (int i = 0; i < 9; i += 1) {
			int neighborX = currentPoint.x + neighborOffsets[i][0];
			int neighborY = currentPoint.y + neighborOffsets[i][1];
			// if the given neighbor pixel is in bounds, let's get its values, multiply them by 
			// their weight (as provided by the kernel), then accumulate the values
			if (checkImageBounds(neighborX, neighborY, 640, 480)) {
				
				Pixel neighbor = getPixel(framebuffer, neighborX, neighborY, 640);


				r += int(neighbor.r) * convolutionMatrix[i];
				g += int(neighbor.g) * convolutionMatrix[i];
				b += int(neighbor.b) * convolutionMatrix[i];

				
			}
			// if the neighbor pixel is out of image bounds, we'll do "EXTEND" edge mode
			// handling - we clamp the out of bounds point to the image edge and then
			// get our neighbor pixel, weight it and accumulate it
			else {
				Point clamped = clampToImageEdges(neighborX, neighborY, 640, 480);

				Pixel neighbor = getPixel(framebuffer, clamped.x, clamped.y, 640);

				r += int(neighbor.r) * convolutionMatrix[i];
				g += int(neighbor.g) * convolutionMatrix[i];
				b += int(neighbor.b) * convolutionMatrix[i];
			}


			//std::cout << offset << std::endl;
			

			//std::cout << "Output pixel, R: " << int(output.r) << " G: " << int(output.g) << " B: " << int(output.b) << " A: " << int(output.a) << std::endl;


		}


			// a box blur kernel convolution requires a normalization, so here's where 
			// we normalize by dividing by 9

			r /= 9;
			g /= 9;
			b /= 9;


			// YOU MAY NOT HAVE TO CLAMP THESE RESULTS ^^^^^^

			// create an output pixel by clamping the weighted and accumulated RGB values to unsigned char range
			// note that we use framebuffer[offset + 3] for the alpha value, which just makes the output pixel alpha = the input pixel alpha

			// *** IMPORTANT!!! ***
			// certain compilers may not be up to speed with c++17, which is where we get the std::clamp function...
			// in that case, fall back to the below statement, which uses the slow clamp function i wrote...
			//Pixel output(clamp(r), clamp(g), clamp(b), framebuffer[offset + 3]);
			Pixel output(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), framebuffer[offset + 3]);

			destinationBuffer[offset] = output.r;
			destinationBuffer[offset + 1] = output.g;
			destinationBuffer[offset + 2] = output.b;
			destinationBuffer[offset + 3] = output.a;

	}

	return destinationBuffer;

}



EMSCRIPTEN_KEEPALIVE unsigned char* sobelFeldman(unsigned char framebuffer[]) {
	int neighborOffsets[9][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
	int kernelX[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
	int kernelY[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

	// allocate a destination buffer on the heap
	unsigned char* destinationBuffer = new unsigned char[1228800];

	for (int offset = 0; offset < 1228800; offset += 4) {

		Point currentPoint = offsetToPoint(offset, 640);

		int r = 0;
		int g = 0;
		int b = 0;

		for (int i = 0; i < 9; i += 1) {
			int neighborX = currentPoint.x + neighborOffsets[i][0];
			int neighborY = currentPoint.y + neighborOffsets[i][1];
			// if the given neighbor pixel is in bounds, let's get its values, multiply them by 
			// their weight (as provided by the kernel), then accumulate the values
			if (checkImageBounds(neighborX, neighborY, 640, 480)) {
				
				Pixel neighbor = getPixel(framebuffer, neighborX, neighborY, 640);


				r += int(neighbor.r) * kernelX[i];
				g += int(neighbor.g) * kernelX[i];
				b += int(neighbor.b) * kernelX[i];

				
			}
			// if the neighbor pixel is out of image bounds, we'll do "EXTEND" edge mode
			// handling - we clamp the out of bounds point to the image edge and then
			// get our neighbor pixel, weight it and accumulate it
			else {
				Point clamped = clampToImageEdges(neighborX, neighborY, 640, 480);

				Pixel neighbor = getPixel(framebuffer, clamped.x, clamped.y, 640);

				r += int(neighbor.r) * kernelX[i];
				g += int(neighbor.g) * kernelX[i];
				b += int(neighbor.b) * kernelX[i];
			}

		}


		// if i understand this operation correctly, all we're supposed to do is
		// perform the X convolution and then the Y convolution and then just add
		// the pixel values together, clamp them to byte range and that's our
		// output pixel? so I can just repeat the above process and keep summing
		// into my r,g,b variables?

		for (int i = 0; i < 9; i += 1) {
			int neighborX = currentPoint.x + neighborOffsets[i][0];
			int neighborY = currentPoint.y + neighborOffsets[i][1];
			// if the given neighbor pixel is in bounds, let's get its values, multiply them by 
			// their weight (as provided by the kernel), then accumulate the values
			if (checkImageBounds(neighborX, neighborY, 640, 480)) {
				
				Pixel neighbor = getPixel(framebuffer, neighborX, neighborY, 640);


				r += int(neighbor.r) * kernelY[i];
				g += int(neighbor.g) * kernelY[i];
				b += int(neighbor.b) * kernelY[i];

				
			}
			// if the neighbor pixel is out of image bounds, we'll do "EXTEND" edge mode
			// handling - we clamp the out of bounds point to the image edge and then
			// get our neighbor pixel, weight it and accumulate it
			else {
				Point clamped = clampToImageEdges(neighborX, neighborY, 640, 480);

				Pixel neighbor = getPixel(framebuffer, clamped.x, clamped.y, 640);

				r += int(neighbor.r) * kernelY[i];
				g += int(neighbor.g) * kernelY[i];
				b += int(neighbor.b) * kernelY[i];
			}
			
		}

		Pixel output(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), framebuffer[offset + 3]);

			destinationBuffer[offset] = output.r;
			destinationBuffer[offset + 1] = output.g;
			destinationBuffer[offset + 2] = output.b;
			destinationBuffer[offset + 3] = output.a;

	}

	return destinationBuffer;


}



EMSCRIPTEN_KEEPALIVE unsigned char* boxBlurAndEdgeDetect(unsigned char framebuffer[]) {
	unsigned char* destinationBuffer = boxBlur3x3(framebuffer);

	unsigned char* otherDestinationBuffer = edgeDetect(destinationBuffer);

	delete destinationBuffer;

	return otherDestinationBuffer;
}



int main() {
	std::cout << "Hello world!" << std::endl;

	// the array of unsigned chars below is a 500x500 bitmap of the new york skyline
	// you can uncomment it and use it as the framebuffer for testing purposes

	/*unsigned char framebuffer[1000000] = {
	};*/

	return 0;
}




#ifdef __cplusplus
}
#endif