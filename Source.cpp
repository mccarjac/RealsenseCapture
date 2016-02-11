//OpenCV Includes
//#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/stitching.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

//Realsense RSSDK Inclues
#include <pxcsession.h>
#include <pxcsensemanager.h>
#include <pxccapturemanager.h>
#include "util_render.h"

//Cinder Includes, default namespace ---cinder::	to use gl... cinder::gl...
#include <cinder/gl/texture.h>
#include <cinder/gl/gl.h>
#include <cinder/app/AppNative.h>
#include "CinderOpenCV.h"
#include <cinder/ImageIo.h>
#include <cinder/Capture.h>
//OpenCV Namespace
using namespace cv;
using namespace ci;
using namespace ci::app;
//#include <GLFW/glfw3.h>
//#include <thread>
#include <vector>

struct dubCam
{
	PXCCapture::DeviceInfo one;
	PXCCapture::DeviceInfo two;
};

struct pxcDust
{
	PXCImage * pxcOne;
	PXCImage * pxcTwo;
};

struct myMats
{
	cv::Mat one;
	cv::Mat two;
};

struct myConversion
{
	IplImage *image;
	PXCImage::ImageData data;
	PXCImage::ImageData info;
	unsigned char * rgbData;
	cv::Mat cvImage;
	int height, width, rotAngle;
	cv::Mat rotMat;

};

CvSize gab_size;
cv::Mat myHomoMat;

dubCam getDeviceInfo(PXCSenseManager * mySenseMan);
void CaptureDeviceStream(PXCSenseManager * sm, PXCSenseManager * sm2, PXCImage * color, PXCImage * color2, pxcDust * combo, PXCCapture::Sample * mySample);
int configStreams(PXCSenseManager * senseMan, PXCSenseManager * sm2, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file, pxcCHAR * file2);
void pxcToMat(pxcDust * imgsToConvert, myMats * convertedImages, myConversion * one, myConversion * two);
int renderImages(myMats * imgs);
void calcHomo(myMats * testImgs);
//UtilRender *renderColor = new UtilRender(L"COLOR STREAM");
//UtilRender *renderColor2 = new UtilRender(L"COLOR STREAM");

void main()
{

	char c;

	//Create our SDK Session for Module Management
	PXCSenseManager *sm = PXCSenseManager::CreateInstance();
	PXCSenseManager *sm2 = PXCSenseManager::CreateInstance();

	dubCam currentDevices;
	currentDevices = getDeviceInfo(sm);

	pxcCHAR * firstCam = L"C:/Users/Group34/Desktop/cam1.rssdk";
	pxcCHAR * secCam = L"C:/Users/Group34/Desktop/cam2.rssdk";

	myMats cvImages;
	PXCImage* color = 0, *color2 = 0;
	pxcDust myPXCs = {};

	myConversion camOne = {}, camTwo = {};
	//Universal conversion info///
	gab_size.height = 480;
	gab_size.width = 640;

	camOne.image = cvCreateImage(gab_size, 8, 3);
	camTwo.image = cvCreateImage(gab_size, 8, 3);

	if (configStreams(sm, sm2, currentDevices.one, currentDevices.two, firstCam, secCam) != 0) wprintf_s(L"Error initializing\n");

	myMats convertedImages = {};
	PXCCapture::Sample * samp = 0;
	for (;;)
	{

		CaptureDeviceStream(sm, sm2, color, color2, &myPXCs, samp);
		pxcToMat(&myPXCs, &convertedImages, &camOne, &camTwo);
		if (renderImages(&convertedImages) != 0) break;

	}


	//Keep our session alive as long as we are accessing modules within it
	sm->Release();
	sm2->Release();
	printf_s("Just making sure the programs working\n");
	scanf_s("%c", &c);
}



dubCam getDeviceInfo(PXCSenseManager * mySenseMan)
{
	//Session Module - Create and Query instances of I/O and algorithm implmentations
	PXCCapture::DeviceInfo camOne, camTwo;
	struct dubCam myCameras = {};
	PXCSession::ImplVersion v = mySenseMan->QuerySession()->QueryVersion();

	//Create an ImplDesc to capture our video module implementation
	PXCSession::ImplDesc desc1 = {};
	desc1.group = PXCSession::IMPL_GROUP_SENSOR;
	desc1.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;

	for (int m = 0;; m++)
	{
		PXCSession::ImplDesc desc2 = {};
		if (mySenseMan->QuerySession()->QueryImpl(&desc1, m, &desc2) < PXC_STATUS_NO_ERROR) break;
		wprintf_s(L"Module[%d]: %s\n", m, desc2.friendlyName);

		PXCCapture *captureOne = 0;

		pxcStatus sts = mySenseMan->QuerySession()->CreateImpl<PXCCapture>(&desc2, &captureOne);
		//Create CaptureManager Module.
		if (sts < PXC_STATUS_NO_ERROR) continue;
		PXCCapture::DeviceInfo dinfo;

		for (int d = 0;; d++)
		{
			sts = captureOne->QueryDeviceInfo(d, &dinfo);
			if (d == 2) camOne = dinfo;

			else if (d == 3) camTwo = dinfo;

			if (sts < PXC_STATUS_NO_ERROR) break;
			wprintf_s(L"	Device[%d]: %s\n", d, dinfo.name);
		}
		captureOne->Release();
	}

	myCameras.one = camOne;
	myCameras.two = camTwo;
	return myCameras;//camOne, camTwo;

}

void CaptureDeviceStream(PXCSenseManager * senseMan, PXCSenseManager * sm2, PXCImage * color, PXCImage * color2, pxcDust * combo, PXCCapture::Sample * mySample)
{
	//PXCImage::Rotation rotatedImage;
	if (senseMan->AcquireFrame(false) < PXC_STATUS_NO_ERROR)throw(0);
	mySample = senseMan->QuerySample();
	color = mySample->color;

	if (sm2->AcquireFrame(false) < PXC_STATUS_NO_ERROR)throw(0);
	mySample = sm2->QuerySample();
	color2 = mySample->color;

	combo->pxcOne = color2;
	combo->pxcTwo = color;

	senseMan->ReleaseFrame();
	sm2->ReleaseFrame();

}

void pxcToMat(pxcDust * imgsToConvert, myMats * convertedImages, myConversion * one, myConversion * two)
{



	imgsToConvert->pxcOne->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &one->data);
	imgsToConvert->pxcTwo->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &two->data);

	one->rgbData = one->data.planes[0];
	two->rgbData = two->data.planes[0];
	for (int y = 0; y < 480; y++)
	{
		for (int x = 0; x < 640; x++)
		{
			for (int k = 0; k < 3; k++)
			{
				one->image->imageData[y * 640 * 3 + x * 3 + k] = one->rgbData[y * 640 * 3 + x * 3 + k];
				two->image->imageData[y * 640 * 3 + x * 3 + k] = two->rgbData[y * 640 * 3 + x * 3 + k];

			}
		}
	}
	imgsToConvert->pxcOne->ReleaseAccess(&one->data);
	imgsToConvert->pxcTwo->ReleaseAccess(&two->data);
	one->cvImage = cv::cvarrToMat(one->image);
	one->height = one->cvImage.rows / 2.;
	one->width = one->cvImage.cols / 2.;
	one->rotAngle = 180;
	one->rotMat = getRotationMatrix2D(Point(one->width, one->height), (one->rotAngle), 1);
	cv::warpAffine(one->cvImage, one->cvImage, one->rotMat, one->cvImage.size());
	two->cvImage = cv::cvarrToMat(two->image);

	convertedImages->one = one->cvImage;
	convertedImages->two = two->cvImage;

}

int configStreams(PXCSenseManager * senseMan, PXCSenseManager * sm2, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file, pxcCHAR * file2)
{
	pxcDust testPxcs = {};
	myMats testMats = {};
	PXCCaptureManager * cm = senseMan->QueryCaptureManager();
	PXCCaptureManager * cm2 = sm2->QueryCaptureManager();

	cm->SetFileName(file, false);
	cm2->SetFileName(file2, false);

	senseMan->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);
	sm2->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);

	cm->FilterByDeviceInfo(&dev1);
	cm2->FilterByDeviceInfo(&dev2);

	senseMan->Init();
	sm2->Init();
	if (senseMan->AcquireFrame(false) < PXC_STATUS_NO_ERROR)throw(0);
	PXCCapture::Sample* sample = senseMan->QuerySample();
	testPxcs.pxcOne = sample->color;

	if (sm2->AcquireFrame(false) < PXC_STATUS_NO_ERROR)throw(0);
	PXCCapture::Sample *sample2 = sm2->QuerySample();
	testPxcs.pxcTwo = sample2->color;

	myConversion one, two;

	one.image = cvCreateImage(gab_size, 8, 3);
	two.image = cvCreateImage(gab_size, 8, 3);

	pxcToMat(&testPxcs, &testMats, &one, &two);
	wprintf_s(L"If we don't find ?? you're goood %s\n",testMats.one);
	calcHomo(&testMats);
	senseMan->ReleaseFrame();
	sm2->ReleaseFrame();

	return 0;

}

void calcHomo(myMats * testImgs)
{

	std::vector<KeyPoint> objKeys;
	std::vector<KeyPoint>  sceneKeys;
	cv::Mat greyOne, greyTwo;

	Ptr<xfeatures2d::SURF> surf = xfeatures2d::SURF::create(400);

	//cv::Mat desc_obj, desc_scene;
	cv::cvtColor(testImgs->one, greyOne, CV_RGB2GRAY);
	cv::cvtColor(testImgs->two, greyTwo, CV_RGB2GRAY);

	cv::Mat descObj, sceneObj;

	surf->detectAndCompute(testImgs->one, Mat(), objKeys, descObj);
	surf->detectAndCompute(testImgs->two, Mat(), sceneKeys, sceneObj);

	FlannBasedMatcher matcher;
	std::vector<DMatch> matches;
	matcher.match(descObj, sceneObj, matches);

	double maxDist = 0, minDist = 100;

	std::vector<DMatch> good_matches;

	for (int i = 0; i < descObj.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < minDist) minDist = dist;
		if (dist > maxDist) maxDist = dist;
	}

	for (int i = 0; i < descObj.rows; i++)
	{
		if (matches[i].distance < (3 * minDist))
		{
			good_matches.push_back(matches[i]);
		}
	}
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;

	for (int i = 0; i < good_matches.size(); i++)
	{
		obj.push_back(objKeys[good_matches[i].queryIdx].pt);
		scene.push_back(sceneKeys[good_matches[i].trainIdx].pt);
	}

	cv::Mat myHomoMat = findHomography(obj, scene, CV_RANSAC);
	wprintf_s(L"Calculated homography without errors\n");
}

int renderImages(myMats * imgs)
{

	/*cv::Mat result = Mat();
	wprintf_s(L"1: %s ---2: %s ---3: %s --\n", imgs->one, imgs->two, result);

	warpPerspective(imgs->one, result, myHomoMat, cv::Size(imgs->one.cols + imgs->two.cols, imgs->one.rows));
	cv::Mat half(result, cv::Rect(0, 0, imgs->two.cols, imgs->two.rows));

	imgs->two.copyTo(half);*/
	cv::imshow("camOne", imgs->one);
	cv::imshow("camTwo", imgs->two);

	if (cvWaitKey(10) >= 0)
	{
		throw(0);
	}

	return 0;
}