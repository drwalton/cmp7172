#include <opencv2/opencv.hpp>
#include <chrono>

/* This will be a brief exercise denoising images using OpenCV, and profiling to find
* which method provides the best quality-performance trade-off.
* 
* The existing code loads an image with a substantial amount of noise, and attempts to denoise it using
* 3 different methods. I'd like you to time the functions using the std::chrono library. Make sure to
* use the right clock!
* 
* The 3 denoising methods are:
* 1. Gaussian blur (use a 9x9 kernel, with standard deviation 4 in both directions)
* 2. Bilateral Filter (use a 9x9 kernel again, with color standard deviation 100 and spatial standard deviation 5)
* 3. Median filtering (use a 5x5 kernel)
* 
* Which method provides the best denoising for the compute cost?
* Remember to test timing in Release rather than Debug mode.
* If you have time, you can try adjusting these parameters to improve the results further.
*/

int main()
{
	cv::Mat noisyImage = cv::imread("../images/car_noisy.jpg");

	cv::Mat gaussBlurImage, bilateralImage, medianFilterImage;

	// -- Your code here --
	// Filter the image with a 9x9 gaussian filter, stdev 4 in both directions
	// time the filtering function and print the elapsed time to the console.

	cv::GaussianBlur(noisyImage, gaussBlurImage, cv::Size(9, 9), 4.f, 4.f);
	cv::imwrite("../images/car_gaussblur.jpg", gaussBlurImage);

	// -- Your code here --
	// Filter the image with a 9x9 bilateral filter, color stdev 100, spatial 5
	// time the filtering function and print the elapsed time to the console.

	cv::bilateralFilter(noisyImage, bilateralImage, 9, 100.f, 5.f);
	cv::imwrite("../images/car_bilateral.jpg", bilateralImage);

	// -- Your code here --
	// Filter the image with a 5x5 median filter
	// time the filtering function and print the elapsed time to the console.


	cv::medianBlur(noisyImage, medianFilterImage, 5);
	cv::imwrite("../images/car_median.jpg", medianFilterImage);


	return 0;
}
