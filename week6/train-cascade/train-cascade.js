const fs = require("fs");
const Canvas = require("canvas");

const Module = require("./wasm-cv.js");

const pathToPositiveImages = process.argv[2];
const pathToNegativeImages = process.argv[3];

global.positiveImages = [];
global.negativeImages = [];
global.canvas = new Canvas(24, 24);
global.ctx = canvas.getContext("2d");
global.img = new Canvas.Image();

if (!fs.existsSync(pathToPositiveImages)) {
	console.log("Error: path '" + pathToPositiveImages + "' not found!");
} else {
	console.log("Wasmface");
	console.log("Cascade classifier training\n");
	console.log("Performing recursive file scan for path '" + pathToPositiveImages + "'");
	console.log("(I sure hope there are only .jpg and/or .png files in there)");
	setTimeout(() => {
		walk(pathToPositiveImages, positiveImages);
		walk(pathToNegativeImages, negativeImages);
		console.log("\nPositive image files found: " + positiveImages.length);
		console.log("Negative image files found: " + negativeImages.length);
		Module.ccall("trainCascade", null, ["number", "number"], [positiveImages.length, negativeImages.length]);
	}, 2000)
}

function walk(path, arr) {
	if (fs.lstatSync(path).isDirectory()) {
		const childPaths = fs.readdirSync(path);
		childPaths.forEach(childPath => walk(path + "/" + childPath, arr));
	} else {
		processFile(path, arr);
	}
}

function processFile(path, arr) {
	if (path.endsWith("png") || path.endsWith("jpg")) {
		console.log("\nFound file --> " + path);
    	arr.push(path);
	}
}