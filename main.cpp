// test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
#define THRESHOLD 35
#define QUIT 113
#define GRAYSCALE 103
#define COMPLEMENT 105
#define USB_CAREMA 99
#define CATCH_BG 99
#define FRONTGROUND 103
#define BACKGROUND_SUB 98
#define BINARY_THRESHOLD 116
#define HISTOGRAM 104
#ifdef _DEBUG
#pragma comment(lib,"opencv_world460d.lib")
#else
#pragma comment(lib,"opencv_world460.lib")
#endif // DEBUG
Mat frame, gray_frame, sub_frame;

int g_nTrackbarValue;
int g_nKernalValue;
const int g_nTrackbarMax = 9;
void ToGray(Mat &input, Mat &output)
{
	for (int i = 0; i < input.rows; i++)
	{
		for (int j = 0;j < input.cols; j++)
		{
			output.at<uchar>(i, j) = saturate_cast<uchar>(0.114*input.at<Vec3b>(i, j)[0] +0.587*input.at<Vec3b>(i, j)[1] + 0.299*input.at<Vec3b>(i, j)[2]);
		}
	}
}
Mat ToComplement(Mat input)
{
	const int nrows = input.rows;
	const int ncols = input.cols;
	Mat result = Mat::zeros(input.size(), input.type());
	for (int i = 0;i < nrows; i++)
	{
		for (int j = 0; j < ncols;j++)
		{
			for (int k = 0; k < 3; k++)
			{
				result.at<Vec3b>(i, j)[k] = saturate_cast<uchar>(-1 * (input.at<Vec3b>(i, j)[k]) + 255);
			}
		}
	}
	return result;
}
void BinaryThreshold(Mat &sub_Frame)
{
	for (int i = 0; i < sub_Frame.rows; i++)
	{
		for (int j = 0; j < sub_Frame.cols; j++)
		{
			if (sub_Frame.at<uchar>(i, j) > THRESHOLD)
			{
				sub_Frame.at<uchar>(i, j) = 255;
			}
			else
			{
				sub_Frame.at<uchar>(i, j) = 0;
			}
		}
	}
}
void on_kernalTractbar(int, void*) {
	g_nKernalValue = g_nKernalValue * 2 + 1;
	blur(frame, gray_frame, Size(g_nKernalValue, g_nKernalValue));
	imshow("aaa", gray_frame);
}
int main()
{
	VideoCapture capture("flyman512x512.avi");
	VideoCapture usbCap(0, CAP_DSHOW);
	Mat camera_bg_frame=Mat::zeros(3,3,CV_8UC3);


	Mat background=imread("flymanBG.jpg");
	Mat gray_bg = imread("flymanBG.jpg", 0);
	Point minLoc, maxLoc;
	double minVal = 0, maxVal = 0;
	bool end_flag=0;
	int bins = 256;
	int hist_size[] = { bins };
	float range[] = { 0,256 };
	const float* ranges[] = { range };
	Mat hist;
	int channels[] = { 0 };
	int scale = 2;
	int hist_height = 256;
	Mat hist_img = Mat::zeros(hist_height, bins*scale, CV_8UC3);
	bool camera_catch = 0,catch_flag=1;
	bool camera_front = 0;
	bool quit_camera = 0;
	char kernalName[20];
	g_nKernalValue = 1;
	namedWindow("aaa", WINDOW_AUTOSIZE);
	createTrackbar(kernalName, "aaa", &g_nTrackbarValue, g_nTrackbarMax, on_kernalTractbar);
	while (1)
	{
		switch (waitKey(30))
		{
		case QUIT://q quit
			end_flag = 1;
			break;
		case GRAYSCALE://g gray video
			while (capture.read(frame)) {
				gray_frame.create(frame.rows, frame.cols, CV_8UC1);// create saving obj.
				ToGray(frame, gray_frame);
				imshow("aaa", gray_frame);
				if (waitKey(5) >= 30) {
					break;
				}
			}
			break;
		case COMPLEMENT://i Complement video
			while (capture.read(frame)) {
				Mat out=ToComplement(frame);
				imshow("aaa", out);
				if (waitKey(5) >= 30) {
					break;
				}
			}			
			break;
		case USB_CAREMA://c  USB camera
			while (usbCap.read(frame))
			{
				
				switch (pollKey())
				{
				case CATCH_BG: // c catch background
					camera_catch = 1;
					break;
				case FRONTGROUND:// g front
					camera_front = 1;
					break;
				case QUIT: // q quit camera
					quit_camera = 1;
					break;
				default:
					break;
				}
				if (camera_front == 1)// calculate fg
				{
					imshow("bg", camera_bg_frame);
					absdiff(frame, camera_bg_frame, sub_frame);
					imshow("aaa", sub_frame);
					if (waitKey(30) >= 30) {
						break;
					}
				}
				else if (camera_catch == 1&& catch_flag ==1)// read bg
				{
					usbCap.read(camera_bg_frame);
					catch_flag = 0;
				}
				else if (quit_camera == 1)// quit camera mode
				{
					break;
				}
				else {
					imshow("aaa", frame);
				}
			}
			if (camera_catch==1)
				destroyWindow("bg");
			camera_catch = 0;
			catch_flag = 1;
			camera_front = 0;
			quit_camera = 0;
			break;
		case BACKGROUND_SUB://b background subtraction
			while (capture.read(frame)) {
				absdiff(frame, background, sub_frame);
				imshow("aaa", sub_frame);
				if (waitKey(30) >= 30) {
					break;
				}
			}
			break;
		case BINARY_THRESHOLD://t binary thresholding
   			while (capture.read(frame))
			{
				gray_frame.create(frame.rows, frame.cols, CV_8UC1);// create saving obj.
				ToGray(frame, gray_frame);
				absdiff(gray_frame, gray_bg, sub_frame);
				minMaxLoc(sub_frame, &minVal, &maxVal, &minLoc, &maxLoc);
				sub_frame = sub_frame / maxVal * 255;
				BinaryThreshold(sub_frame);
				imshow("aaa", sub_frame);
				if (waitKey(30) >= 30) {
					break;
				}
			}
			break;
		case HISTOGRAM://h histogram
			while (capture.read(frame))
			{
				cvtColor(frame, gray_frame, COLOR_RGB2GRAY);
				calcHist(&gray_frame, 1, channels, Mat(), hist, 1, hist_size, ranges, true, false);
				minMaxLoc(hist, 0, &maxVal, 0, 0);
				scale = 2;
				hist_height = 256;
				hist_img = Mat::zeros(hist_height, bins*scale, CV_8UC3);
				for (int i = 0; i < bins; i++)
				{
					float binval = hist.at<float>(i);
					int intensity = cvRound(binval / maxVal * hist_height);
					rectangle(hist_img, Point(i*scale, hist_height - 1), Point((i + 1)*scale - 1, hist_height - intensity), CV_RGB(255, 255, 255));
				}
				imshow("hist", hist_img);
				imshow("aaa", frame);
				if (waitKey(30) >= 30) {
					destroyWindow("hist");
					break;
				}
			}
			break;
		case 97:
			//capture.read(frame);
			
				//gray_frame.create(frame.rows, frame.cols, CV_8UC1);// create saving obj.
				on_kernalTractbar(g_nKernalValue, 0);
				//g_nKernalValue = 0;
				//g_nTrackbarValue = 1;
				waitKey(0);
				
			
			
			break;
		default:// default run normal pic
			g_nKernalValue = 0;
			g_nTrackbarValue = 1;
			capture>>frame;
			if (frame.empty())
			{
				cout << "end of video" << endl;
				return 0;
			}
			imshow("aaa", frame);

			break;
		}
		if (end_flag == 1)
			break;
	}
	return 0;
}

