#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <Eigen/Dense>

/* Practice using OpenCV
 * This includes a couple of demos showing how to load images
 */

void loadShowImageDemo()
{
	// OpenCV has a really nice simple interface for loading & displaying images
	// Imread loads the image into OpenCV's container class cv::Mat
	cv::Mat carImage = cv::imread("../images/car.jpg");

	// Let's print some info about the image we've loaded:
	std::cout << "Image size: " << carImage.cols << "x" << carImage.rows << std::endl;

	// cv::imshow is an easy function to show an image in a window
	// useful for debugging and checking you've loaded your textures properly!
	// You give it a string as the first argument which is the title of the window
	// 
	cv::imshow("Car Image", carImage);

	// cv::waitKey() is required after cv::imshow() to actually show the image.
	// By default it waits forever, until a key is pressed.
	// If you call with a number e.g. cv::waitKey(1) it will wait for 1ms for a key
	// and then return.
	cv::waitKey();

	// Saving images is just as easy:
	cv::imwrite("car.png", carImage);
	// This will probably have saved the image to your build/ directory.
}

void loadImageTransparencyDemo()
{
	//By default OpenCV loads everything as a BGR image

	cv::Mat invaderImage = cv::imread("../images/invader.png");

	std::cout << "Number of channels: " << invaderImage.channels() << std::endl;

	// This is 3, even though I know this image happens to be 4-channel texture with alpha
	// To load a 4-channel image in 4-channel mode (or e.g. a grayscale image in 1-channel mode)
	// use cv::IMREAD_UNCHANGED

	invaderImage = cv::imread("../images/invader.png", cv::IMREAD_UNCHANGED);

	std::cout << "Number of channels: " << invaderImage.channels() << std::endl;
}

void imageCaptureDemo()
{
	// OpenCV has an easy system for loading frames from a camera
	// or video file.
	cv::VideoCapture capture;
	capture.open(0); // Opens the first connected camera (1 will open the 2nd)

	int pressedKey = 0; 
	cv::Mat camImage;

	// Keep showing frames until the q key is pressed.
	while (pressedKey != 'q') {
		capture >> camImage;
		cv::imshow("Live camera image", camImage);
		pressedKey = cv::waitKey(1);
	}
}

void ex1ConvertImageColor()
{
	cv::Mat carImage = cv::imread("../images/car.png");

	// Convert this image to grayscale, and display it using cv::imshow()
	// You might want to make use of cv::cvtColor().
}

void ex2ResizeImage()
{
	cv::Mat carImage = cv::imread("../images/car.png");

	// Upsample this image 2x with cv::resize().
	// Try the different modes for interpolation - how are
	// cv::INTER_NEAREST and cv::INTER_CUBIC different?
}

void ex3SharpenImage()
{
	// This loads and shows a grayscale image of a bus
	// Note this image is pretty blurry!
	// For this exercise, we'll try sharpening the image, using unsharp masking
	// If you blur an image and subtract, then
	// hifreq = image - blurred 
	// contains all the high-frequency detail.
	// If you then compute 
	// blurred + hifreq * alpha
	// for some value of alpha bigger than 1, this will give a sharpened result.
	// Do this to enhance the image - try some different values of alpha
	// HINTS
	// * Try cv::GaussianBlur to do the blur
	// * The input image is in 8-bit unsigned format, so you can't really subtract
	//    & multiply easily without overflow issues etc. You might get better results if you
	//    convert the image to CV_32FC1 format (32bit, floating-point, 1 channel)

	cv::Mat busImage = cv::imread("../images/bus.png", cv::IMREAD_UNCHANGED);
	cv::imshow("busImage", busImage);


	cv::Mat busImageEnhanced;
	// TODO set busImageEnhanced to be an unsharp masked version of busImage
	cv::imshow("busImageEnhanced", busImageEnhanced);
	cv::waitKey();

}

void ex4HeightmapToNormalmap()
{
	// Now let's do something a big more graphics-y
	// One way of producing a normal map for improved lighting detail
	// on an object is by starting from a heightmap. 
	// Differentiating the heightmap along the x and y directions 
	// gives you tangents to the surface.
	// You can take the cross product of these to get a normal.

	// Calculate a normal map from the heightmap loaded below in this way
	// Save it to a file in a way that it can be loaded, and the normal
	// map recovered. An example of the output you should expect is at 
	// images/normalmap.png

	// Hints
	// -----
	// 1. The normal way to calculate image gradients is by filtering with
	// a gradient kernel like Sobel (cv::Sobel)
	// 2. When saving to an image, remember that images can't generally store
	// negative values, so you'll have to remap your normals from the [-1,1] 
	// range to the [0,1] range.
	// 3. If your normal map is in a weird colour, remember that OpenCV images
	// are stored in BGR format (so if you want z to be in the blue channel,
	// it should come first). You can use cv::cvtColor to convert to BGR if you'd
	// like.

	cv::Mat heightMap8bit = cv::imread("../images/heightmap.png", cv::IMREAD_UNCHANGED);
	cv::Mat heightMap;

	heightMap8bit.convertTo(heightMap, CV_32F);
	heightMap /= 255.f;

	// Your code here! Compute the normal map, and save it.
}

int main()
{
	loadShowImageDemo();

	//loadImageTransparencyDemo();

	//imageCaptureDemo();

	//ex1ConvertImageColor();

	//ex2ResizeImage();

	//ex3SharpenImage();

	//ex4HeightmapToNormalmap();

	return 0;
}

