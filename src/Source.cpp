#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
using namespace std;
using namespace cv;


int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	std::vector<cv::String> fileNames;
	cv::glob("data/*.png", fileNames, false); //taking checkerboard images
	std::vector<std::vector<cv::Point2f>> foundPoints(fileNames.size());


	std::vector<std::vector<cv::Point3f>> worldCornerPoints;

										// SECTION 1   Create Known CheckerBoard Positions
	//The board has 25 x 18
	// fields with a size of 15x15mm
	int checkerBoard[2] = { 18,25 };
	std::vector<cv::Point3f> corners; 

	for (int i = 1; i < checkerBoard[1]; i++) {
		for (int j = 1; j < checkerBoard[0]; j++) {
			corners.push_back(cv::Point3f(j, i, 0)); // push to the corner vector all of the calculated locations where we expect the things to be
		}
	}

						// END OF SECTION 1

		
				// SECTION 2
	//Get ChessBoard Corners
	std::vector<cv::Point2f> imgPoint;
	// Detect feature points
	std::size_t i = 0;
	for (auto const &f : fileNames) {
		std::cout << std::string(f) << std::endl;

		// Read in the image and call cv::findChessboardCorners()
		cv::Mat img = cv::imread(fileNames[i]);
		cv::Mat gray;

		cv::cvtColor(img, gray, cv::COLOR_RGB2GRAY);

		bool patternFound = cv::findChessboardCorners(gray, Size(24,17), foundPoints[i], cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK); //the suitable flags for pattern found
		
		// Use cv::cornerSubPix() to refine the found corner detections
		if (patternFound) {
			cv::cornerSubPix(gray, foundPoints[i], cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
			worldCornerPoints.push_back(corners);
		}

		// Display   If you would like to display, please rmove the comment
		cv::drawChessboardCorners(img, Size(24,18),foundPoints[i], patternFound);
		//cv::imshow("chessboard detection", img);
		//cv::waitKey(0);

		i++;
	}
					// END OF SECTION 2



				// SECTION 3  CAMERA CALIBRATION

	cv::Matx33f intrinsic(cv::Matx33f::eye());  // intrinsic camera matrix
	cv::Vec<float, 5> distortion(0, 0, 0, 0, 0); // distortion parameters

	std::vector<cv::Mat> Rvector, Tvector;
	std::vector<double> stdIntrinsics, stdExtrinsics, perViewErrors;
	int flags = cv::CALIB_FIX_ASPECT_RATIO + cv::CALIB_FIX_K3 +
		cv::CALIB_ZERO_TANGENT_DIST + cv::CALIB_FIX_PRINCIPAL_POINT;

	cv::Size frameSize(2000, 1500);

	std::cout << "Calibrating..." << std::endl;
	// cv::calibrateCamera()" with the input coordinates and output parameters as declared above

	float error = cv::calibrateCamera(worldCornerPoints, foundPoints, frameSize, intrinsic, distortion, Rvector, Tvector, flags);

	std::cout << "Reprojection error = " << error << "\nIntrinsic Parameters =\n"
		<< intrinsic << "\nDistortion Parameters=\n"
		<< distortion << std::endl;

	// Precompute lens correction interpolation
	cv::Mat mapX, mapY;
	cv::initUndistortRectifyMap(intrinsic, distortion, cv::Matx33f::eye(), intrinsic, frameSize, CV_32FC1,
		mapX, mapY);


				//END OF SECTION 5




	cv::Mat img = cv::imread("test_image.png", cv::IMREAD_COLOR);
	cv::Mat imgUndistorted;
	// 5. Remap the image using the precomputed interpolation maps.
	cv::remap(img, imgUndistorted, mapX, mapY, cv::INTER_LINEAR);

	// Display
	cv::namedWindow("Original distorted image",WINDOW_NORMAL);
	cv::imshow("Original distorted image", img);

	cv::namedWindow("Undistorted image",WINDOW_NORMAL);
	cv::imshow("Undistorted image", imgUndistorted);
	cv::waitKey(0);


	return 0;
}