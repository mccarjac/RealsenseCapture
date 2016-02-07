//OpenCV Includes
//#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/core.hpp>

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

struct dubCam
{
	PXCCapture::DeviceInfo one;
	PXCCapture::DeviceInfo two;
};

dubCam getDeviceInfo(PXCSenseManager * mySenseMan);
void CaptureDeviceStream(PXCSenseManager * sm, PXCSenseManager * sm2, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file, pxcCHAR * file2);
void pxcToMat(PXCImage * img,Mat* myCV);

UtilRender *renderColor = new UtilRender(L"COLOR STREAM");
UtilRender *renderColor2 = new UtilRender(L"COLOR STREAM");

void main()
{

char c;

//Create our SDK Session for Module Management
PXCSenseManager *sm = PXCSenseManager::CreateInstance();
PXCSenseManager *sm2 = PXCSenseManager::CreateInstance();

struct dubCam currentDevices;
//Lets get our current devices that are plugged in
currentDevices = getDeviceInfo(sm);

pxcCHAR * firstCam = L"C:/Users/Group34/Desktop/cam1.rssdk";
pxcCHAR * secCam = L"C:/Users/Group34/Desktop/cam2.rssdk";

//For now we just capture the RGB streams from each camera but can easily add functionalities to mess with depth and IR
CaptureDeviceStream(sm,sm2, currentDevices.one,currentDevices.two, firstCam,secCam);




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

void CaptureDeviceStream(PXCSenseManager * senseMan, PXCSenseManager * sm2, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file, pxcCHAR * file2)
{
	//PXCImage::Rotation rotatedImage;
	

	PXCImage * color, *color2;
	PXCCaptureManager * cm = senseMan->QueryCaptureManager();
	//PXCCaptureManager * cm2 = sm2->QueryCaptureManager();
	
	cm->SetFileName(file, false);
	//cm2->SetFileName(file2, false);
	
	senseMan->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);
	//sm2->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);
		
	cm->FilterByDeviceInfo(&dev1);
	//cm2->FilterByDeviceInfo(&dev2);
	
	senseMan->Init();
	sm2->Init();
	
	for (int i = 0; i < 1000; i++)
	{
	
		if (senseMan->AcquireFrame(false) < PXC_STATUS_NO_ERROR)break;
		PXCCapture::Sample * sample = senseMan->QuerySample();
		color = sample->color;

	/*	if (sm2->AcquireFrame(true) < PXC_STATUS_NO_ERROR)break;
		PXCCapture::Sample * sample2 = sm2->QuerySample();
		color2 = sample2->color;
		*/
		cv::Mat matOne, matTwo;
		//CONVERT PXCImage to OpenCV Mat format...
		pxcToMat(color, &matOne);

		senseMan->ReleaseFrame();
		sm2->ReleaseFrame();
	}

	
}

void pxcToMat(PXCImage * img,Mat* myCV)
{

	IplImage * image = 0;
	CvSize gab_size;
	gab_size.height = 480;
	gab_size.width = 640;
	
	image = cvCreateImage(gab_size, 8, 3);

	PXCImage::ImageData data;
	PXCImage::ImageInfo rgbInfo;
	unsigned char *rgbData;

	img->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::PIXEL_FORMAT_RGB24, &data);

	rgbData = data.planes[0];

	for (int y = 0; y < 480; y++ )
		{
			for (int x = 0; x < 640;x++)
				{
					for (int k = 0; k < 3; k++)
					{
						image->imageData[y * 640 * 3 + x * 3 + k] = rgbData[y * 640 * 3 + x * 3 + k];
					}
			}
	}
	img->ReleaseAccess(&data);
	
	cv::Mat newImage = cv::cvarrToMat(image);
	cv::imshow("camOne", newImage);
	if (cvWaitKey(10)>= 0)
	{
		throw(0);
	}

	/*	cinder::gl::Texture mLabelMap, mLabelMap2;
	IplImage * imageOne = cvCreateImageHeader(cvSize(640, 480), 8, 3);
	IplImage * imageTwo = cvCreateImageHeader(cvSize(640, 480), 8, 3);
		
	cvSetData(imageOne, (uchar*)dat1.planes[0], 640 * 3 * sizeof(uchar));
	cvSetData(imageTwo, (uchar*)dat2.planes[0], 640 * 3 * sizeof(uchar));

	cv::Mat newImageOne = cv::cvarrToMat(imageOne);
	cv::Mat newImageTwo = cv::cvarrToMat(imageTwo);
	cv::imwrite("C:/Users/Group34/Desktop/pleaseBeMe.jpg",newImageOne);
	
	mLabelMap = cinder::gl::Texture(fromOcv(newImageOne));
	mLabelMap2 = cinder::gl::Texture(fromOcv(newImageOne));
	
	float w1 = mLabelMap.getWidth();
	float h1 = mLabelMap.getHeight();
	float w2 = mLabelMap2.getWidth();
	float h2 = mLabelMap2.getHeight();

	gl::draw(mLabelMap, Rectf(0, 0, 200,200));
	gl::draw(mLabelMap2, Rectf(0, 0, 200, 200));

	cv::namedWindow("Cam One", WINDOW_AUTOSIZE);
	cv::namedWindow("Cam Two", WINDOW_AUTOSIZE);
	wprintf_s(L"Didn't show em bra\n");
	cv::imshow("Cam One", newImageOne);
	cv::imshow("Cam Two", newImageTwo);
	waitKey(0);	*/
}
