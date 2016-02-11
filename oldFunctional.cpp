/*#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <pxcsession.h>
#include <pxcsensemanager.h>
#include <pxccapturemanager.h>
#include <thread>
#include "util_render.h"

using namespace cv;

void CaptureDeviceStream(PXCSenseManager * sm, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file);
UtilRender *renderColor = new UtilRender(L"COLOR STREAM");
UtilRender *renderDepth = new UtilRender(L"DEPTH STREAM");

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

		for (int d = 0;; d++)
		{
			//PXCCaptureManager *cm = sm->QueryCaptureManager();
			//Query the device?
			PXCCapture::DeviceInfo dinfo;

			sts = captureOne->QueryDeviceInfo(d, &dinfo);
			if (d == 2) camOne = dinfo;
			if (d == 3) camTwo = dinfo;
			if (sts < PXC_STATUS_NO_ERROR) break;
			wprintf_s(L"	Device[%d]: %s\n", d, dinfo.name);
		}
		captureOne->Release();
	}
	pxcCHAR * firstCam = L"C:/Users/Group34/Desktop/cam1.rssdk";
	pxcCHAR * secCam = L"C:/Users/Group34/Desktop/cam2.rssdk";



	//Lets try to kick off two threads that simaeltaneously grab the camera streams..
	CaptureDeviceStream(sm, camOne, camTwo, firstCam);

	wprintf_s(L"After\n");
	//Keep our session alive as long as we are accessing modules within it
	sm->Release();

	printf_s("Just making sure the programs working\n");
	scanf_s("%c", &c);
}


void CaptureDeviceStream(PXCSenseManager * senseMan, PXCCapture::DeviceInfo dev1, PXCCapture::DeviceInfo dev2, pxcCHAR * file)
{
	PXCImage * color;
	senseMan->QueryCaptureManager()->SetFileName(file, false);
	senseMan->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 0);
	PXCCaptureManager * cm = senseMan->QueryCaptureManager();
	wprintf_s(L"Well we're running device %s capturing to file %s..\n", dev1.name, file);

	senseMan->Init();

	for (int i = 0; i < 1000; i++)
	{
		cm->FilterByDeviceInfo(&dev1);
		if (senseMan->AcquireFrame(true) < PXC_STATUS_NO_ERROR)break;
		PXCCapture::Sample * sample = senseMan->QuerySample();
		color = sample->color;

		if (!renderColor->RenderFrame(color))break;

		senseMan->ReleaseFrame();
	}
	wprintf_s(L"Before\n");

}*/