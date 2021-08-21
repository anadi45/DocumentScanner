#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>

using namespace cv;
using namespace std;

Mat img,imgGray,imgBlur,imgCanny,imgDil,imgThres,imgWarp,imgCrop;
vector<Point> initialPoints,docPoints;

float w = 420, h = 596;

Mat preProcessing(Mat img)
{
	GaussianBlur(img, imgBlur, Size(5, 5),7,0);
	Canny(imgBlur, imgCanny, 25,50);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);

	return imgDil;
}

vector<Point> getContours(Mat img)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point> biggest;

	findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());
	
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		cout << area << endl;
		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			
			if (area > maxArea && conPoly[i].size() == 4)
			{
				drawContours(img, conPoly, i, Scalar(255, 0, 255), 2);
				biggest = { conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3] };
				maxArea = area;
			}

			
			//rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
		}
	}
	return biggest;
}

void drawPoints(vector<Point> points, Scalar color)
{
	for (int i = 0; i < points.size(); i++)
	{
		circle(img, points[i], 10, color, FILLED);
		putText(img, to_string(i), points[i], FONT_HERSHEY_COMPLEX,2, color, 2);
	}
}

vector<Point> reOrder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int> sumPoints,subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}

	
	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
	

	return newPoints;
}

Mat getWarp(Mat img, vector<Point>points, float w, float h)
{
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}

int main()
{
	string path = "Resources/paper.jpg";
	img = imread(path);

	resize(img, img, Size(1000, 700));

	cvtColor(img, imgGray,COLOR_BGR2GRAY);

	//Preprocessing of image
	imgThres=preProcessing(img);

	//Get Contours-Biggest one
	initialPoints=getContours(imgThres);
	//drawPoints(initialPoints, Scalar(0, 0, 255));
	docPoints = reOrder(initialPoints);
	//drawPoints(docPoints, Scalar(0, 0, 255));

	//Warp
	imgWarp = getWarp(img,docPoints,w,h);

	//Crop
	int CropVal = 5;
	Rect roi(CropVal, CropVal, w - (2 * CropVal), h - (2 * CropVal));
	imgCrop = imgWarp(roi);

	imshow("Image", img);
	imshow("Final Image", imgCrop);
	//imshow("Grayscale Image", imgGray);
	//imshow("Processed Image", imgThres);;
	
	waitKey(0);
	return 0;
}