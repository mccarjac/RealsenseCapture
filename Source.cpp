#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <pxcsession.h>
#include <pxcsensemanager.h>
#include <pxccapturemanager.h>
#include <thread>
#include "util_render.h"

using namespace cv;

void CaptureDeviceStream(PXCSenseManager * sm, PXCSenseManager * sm2, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file, pxcCHAR * file2);
UtilRender *renderColor = new UtilRender(L"COLOR STREAM");
UtilRender *renderColor2 = new UtilRender(L"COLOR STREAM");

void main()
{

char c;
PXCCapture::DeviceInfo camOne, camTwo;

//Create our SDK Session for Module Management
PXCSenseManager *sm = PXCSenseManager::CreateInstance();
PXCSenseManager *sm2 = PXCSenseManager::CreateInstance();

//Session Module - Create and Query instances of I/O and algorithm implmentations
PXCSession::ImplVersion v = sm->QuerySession()->QueryVersion();

//Create an ImplDesc to capture our video module implementation
PXCSession::ImplDesc desc1 = {};
desc1.group = PXCSession::IMPL_GROUP_SENSOR;
desc1.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;

for (int m = 0;; m++)
	{
	PXCSession::ImplDesc desc2 = {};
	if (sm->QuerySession()->QueryImpl(&desc1, m, &desc2) < PXC_STATUS_NO_ERROR) break;
	wprintf_s(L"Module[%d]: %s\n", m, desc2.friendlyName);

	PXCCapture *captureOne = 0;

	pxcStatus sts = sm->QuerySession()->CreateImpl<PXCCapture>(&desc2, &captureOne);
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

pxcCHAR * firstCam = L"C:/Users/Group34/Desktop/cam1.rssdk";
pxcCHAR * secCam = L"C:/Users/Group34/Desktop/cam2.rssdk";


//Lets try to kick off two threads that simaeltaneously grab the camera streams..
CaptureDeviceStream(sm,sm2, camOne,camTwo, firstCam,secCam);

wprintf_s(L"After\n");
//Keep our session alive as long as we are accessing modules within it
sm->Release();
sm2->Release();
printf_s("Just making sure the programs working\n");
scanf_s("%c", &c);
}

PXCCapture::DeviceInfo getDeviceInfo(PXCSenseManager * mySenseMan)
{


}

void CaptureDeviceStream(PXCSenseManager * senseMan, PXCSenseManager * sm2, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file, pxcCHAR * file2)
{
	//PXCImage::Rotation rotatedImage;
	
	PXCImage::ImageData firstData,secondData;
	PXCImage::ImageInfo firstInfo, secondInfo;
	PXCImage * color, *color2;
	PXCCaptureManager * cm = senseMan->QueryCaptureManager();
	PXCCaptureManager * cm2 = sm2->QueryCaptureManager();
	
	cm->SetFileName(file, false);
	cm2->SetFileName(file2, false);
	
	senseMan->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);
	sm2->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);

	memset(&firstInfo, 0, sizeof(firstInfo));
	memset(&secondInfo, 0, sizeof(secondInfo));

	firstInfo.width = 640;
	secondInfo.width = 640;

	firstInfo.height = 480;
	secondInfo.height = 480;

	firstInfo.format = PXCImage::PIXEL_FORMAT_RGB32;
	secondInfo.format = PXCImage::PIXEL_FORMAT_RGB32;

	cm->FilterByDeviceInfo(&dev1);
	cm2->FilterByDeviceInfo(&dev2);
	
	senseMan->Init();
	sm2->Init();
	
	for (int i = 0; i < 1000; i++)
	{
	
		if (senseMan->AcquireFrame(false) < PXC_STATUS_NO_ERROR)break;
		PXCCapture::Sample * sample = senseMan->QuerySample();
		color = sample->color;

		if (sm2->AcquireFrame(true) < PXC_STATUS_NO_ERROR)break;
		PXCCapture::Sample * sample2 = sm2->QuerySample();
		color2 = sample2->color;
		
		if (!renderColor->RenderFrame(color2))break;
		if (!renderColor2->RenderFrame(color))break;

		senseMan->ReleaseFrame();
		sm2->ReleaseFrame();
	}

	wprintf_s(L"Before\n");

}
