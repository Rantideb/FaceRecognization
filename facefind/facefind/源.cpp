#include<atlstr.h>
#include <windows.h> 
#include <iostream>
#include "cv.h"
#include "highgui.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp" 

#include "opencv2/objdetect/objdetect.hpp"

using namespace std;
using namespace cv;



void detectAndDisplay(Mat frame);

//--------------------------------【全局变量声明】----------------------------------------------
//		描述：声明全局变量
//-------------------------------------------------------------------------------------------------
//注意，需要把"haarcascade_frontalface_alt.xml"和"haarcascade_eye_tree_eyeglasses.xml"这两个文件复制到工程路径下
String face_cascade_name = "haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "Capture - Face detection";
RNG rng(12345);


//-----------------------------------【main( )函数】--------------------------------------------
//		描述：控制台应用程序的入口函数，我们的程序从这里开始
//-------------------------------------------------------------------------------------------------
int main(void)
{
	VideoCapture capture;
	Mat frame;


	//-- 1. 加载级联（cascades）
	if (!face_cascade.load(face_cascade_name)){ printf("--(!)Error loading\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)){ printf("--(!)Error loading\n"); return -1; };

	//-- 2. 读取视频
	capture.open(0);
	if (capture.isOpened())
	{
		while (1)
		{
			capture >> frame;

			//-- 3. 对当前帧使用分类器（Apply the classifier to the frame）
			if (!frame.empty())
			{
				detectAndDisplay(frame);
			}
			else
			{
				printf(" --(!) No captured frame -- Break!"); break;
			}

			if (waitKey(10) == 27)
			{
				break;
			}

		}
	}
	return 0;
}


void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- 人脸检测
	//此句代码的OpenCV2版为：
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
	//此句代码的OpenCV3版为：
//	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
		ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 2, 8, 0);

		Mat faceROI = frame_gray(faces[i]);
		std::vector<Rect> eyes;

		//-- 在脸中检测眼睛
		//此句代码的OpenCV2版为：
		 eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );
		//此句代码的OpenCV3版为：
//		eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

		for (size_t j = 0; j < eyes.size(); j++)
		{
			Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, eye_center, radius, Scalar(255, 0, 0), 3, 8, 0);
		}
	}
	//-- 显示最终效果图
	imshow(window_name, frame);
}



/*
Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

void onMouse(int event, int x, int y, int, void*)
{
if (selectObject)
{
selection.x = MIN(x, origin.x);
selection.y = MIN(y, origin.y);
selection.width = std::abs(x - origin.x);
selection.height = std::abs(y - origin.y);

selection &= Rect(0, 0, image.cols, image.rows);
}

switch (event)
{
case CV_EVENT_LBUTTONDOWN:
origin = Point(x, y);
selection = Rect(x, y, 0, 0);
selectObject = true;
break;
case CV_EVENT_LBUTTONUP:
selectObject = false;
if (selection.width > 0 && selection.height > 0)
trackObject = -1;
break;
}
}

int main()
{

VideoCapture cap;
Rect trackWindow;
RotatedRect trackBox;
int hsize = 16;
float hranges[] = { 0, 180 };
const float* phranges = hranges;

cap.open("..\\imag\\car.avi");

if (!cap.isOpened())
{

printf("***Could not initialize capturing...***\n");

return -1;
}

namedWindow("Histogram", CV_WINDOW_AUTOSIZE);
namedWindow("CamShift Demo", CV_WINDOW_AUTOSIZE);
setMouseCallback("CamShift Demo", onMouse, 0);
createTrackbar("Vmin", "CamShift Demo", &vmin, 256, 0);
createTrackbar("Vmax", "CamShift Demo", &vmax, 256, 0);
createTrackbar("Smin", "CamShift Demo", &smin, 256, 0);

Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
bool paused = false;

for (;;)
{
if (!paused)
{
cap >> frame;
if (frame.empty())
break;
}

frame.copyTo(image);

if (!paused)
{
cvtColor(image, hsv, CV_BGR2HSV);

if (trackObject)
{
int _vmin = vmin, _vmax = vmax;

inRange(hsv, Scalar(0, smin, MIN(_vmin, _vmax)),
Scalar(180, 256, MAX(_vmin, _vmax)), mask);
int ch[] = { 0, 0 };
hue.create(hsv.size(), hsv.depth());
mixChannels(&hsv, 1, &hue, 1, ch, 1);

if (trackObject < 0)
{
Mat roi(hue, selection), maskroi(mask, selection);
calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
normalize(hist, hist, 0, 255, CV_MINMAX);

trackWindow = selection;
trackObject = 1;

histimg = Scalar::all(0);
int binW = histimg.cols / hsize;
Mat buf(1, hsize, CV_8UC3);
for (int i = 0; i < hsize; i++)
buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / hsize), 255, 255);
cvtColor(buf, buf, CV_HSV2BGR);

for (int i = 0; i < hsize; i++)
{
int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows / 255);
rectangle(histimg, Point(i*binW, histimg.rows),
Point((i + 1)*binW, histimg.rows - val),
Scalar(buf.at<Vec3b>(i)), -1, 8);
}
}

calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
backproj &= mask;
RotatedRect trackBox = CamShift(backproj, trackWindow,
TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
if (trackWindow.area() <= 1)
{
int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
trackWindow.x + r, trackWindow.y + r) &
Rect(0, 0, cols, rows);
}

if (backprojMode)
cvtColor(backproj, image, CV_GRAY2BGR);
ellipse(image, trackBox, Scalar(0, 0, 255), 3, CV_AA);
}
}
else if (trackObject < 0)
paused = false;

if (selectObject && selection.width > 0 && selection.height > 0)
{
Mat roi(image, selection);
bitwise_not(roi, roi);
}

imshow("CamShift Demo", image);
imshow("Histogram", histimg);

char c = (char)waitKey(10);
if (c == 27)
break;
switch (c)
{
case 'b':
backprojMode = !backprojMode;
break;
case 'c':
trackObject = 0;
histimg = Scalar::all(0);
break;
case 'h':
showHist = !showHist;
if (!showHist)
destroyWindow("Histogram");
else
namedWindow("Histogram", 1);
break;
case 'p':
paused = !paused;
break;
default:
;
}
}

return 0;
}
*/
