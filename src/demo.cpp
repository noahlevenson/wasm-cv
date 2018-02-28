#ifdef __cplusplus
extern "C" {
#endif

EMSCRIPTEN_KEEPALIVE unsigned char* morphStack(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	unsigned char* buf1 = new unsigned char[project->size];
	buf1 = toGrayscale(inputBuf, buf1, project);
	unsigned char* buf2 = new unsigned char[project->size];
	outputBuf = binarize(buf1, outputBuf, project);
	delete [] buf1;
	return outputBuf;
}

EMSCRIPTEN_KEEPALIVE unsigned char* ocr(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	unsigned char* buf1 = new unsigned char[project->size];
	buf1 = toGrayscale(inputBuf, buf1, project);
	outputBuf = binarizeOCR(buf1, outputBuf, project);
	delete [] buf1;
	return outputBuf;
}

EMSCRIPTEN_KEEPALIVE unsigned char* medianStack(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	unsigned char* buf1 = new unsigned char[project->size];
	buf1 = toGrayscale(inputBuf, buf1, project);
	outputBuf = rank3x3(buf1, outputBuf, project, 8);
	delete [] buf1;
	return outputBuf;
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoDilate5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return kDilate5x5(inputBuf, outputBuf, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoErode5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return kErode5x5(inputBuf, outputBuf, project, project->se._5x5iso);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoOpen5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return open5x5(inputBuf, outputBuf, project, project->se._5x5disc);
}

EMSCRIPTEN_KEEPALIVE unsigned char* demoClose5x5(unsigned char inputBuf[], unsigned char outputBuf[], Wasmcv* project) {
	return close5x5(inputBuf, outputBuf, project, project->se._5x5disc);
}


#ifdef __cplusplus
}
#endif