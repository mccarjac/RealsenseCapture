/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2015 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#include <Windows.h>
#include "streaming.h"

Streaming::Streaming() :sm(0), mirror(true), main(PXCCapture::STREAM_TYPE_ANY), pip(PXCCapture::STREAM_TYPE_ANY), file(0),
record(false), playback(false), dinfo(), profiles(), synced(true), adaptive(false), running(false), stop(true), thread(INVALID_HANDLE_VALUE)
{
}

Streaming::~Streaming() {
	stop = true;
	while (running) Sleep(5);
	if (thread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(thread);
		thread = INVALID_HANDLE_VALUE;
	}
}

pxcStatus Streaming::OnConnect(PXCCapture::Device *device, pxcBool connected) {
	UpdateStatus(connected ? L"Device Connected" : L"Device Disconnected");
	if (connected) {
		device->SetDeviceAllowProfileChange(adaptive);
		if (sm) {
			/* Enable all selected streams */
			PXCCapture::Device::StreamProfileSet _profiles = {};
			for (PXCCapture::StreamType st = PXCCapture::STREAM_TYPE_COLOR; st != PXCCapture::STREAM_TYPE_ANY; st++)
			{
				PXCCapture::Device::StreamProfile &profile = profiles[st];
				if (!profile.imageInfo.format)
					continue;
				if (adaptive && device->QueryStreamProfileSetNum(st) == 1) {
					device->QueryStreamProfileSet(st, 0, &_profiles);
					profile = _profiles[st];
				}

				/* For simple request, use sm->EnableStream(st, profile.imageInfo.width, profile.imageInfo.height, profile.frameRate.max); */
				PXCVideoModule::DataDesc desc = {};
				desc.streams[st].frameRate.min = desc.streams[st].frameRate.max = profile.frameRate.max;
				desc.streams[st].sizeMin.height = desc.streams[st].sizeMax.height = profile.imageInfo.height;
				desc.streams[st].sizeMin.width = desc.streams[st].sizeMax.width = profile.imageInfo.width;
				desc.streams[st].options = profile.options;
				sm->EnableStreams(&desc);
			}
			sm->QueryCaptureManager()->FilterByStreamProfiles(&profiles);
		}
	}
	return PXC_STATUS_NO_ERROR;
}

void Streaming::Start() {
	stop = false;
	running = true;
	thread = CreateThread(0, 0, ThreadProc, this, 0, 0);
	Sleep(0);
}

void Streaming::Stop() {
	stop = true;
}

DWORD Streaming::ThreadProc(LPVOID arg) {
	((Streaming*)arg)->StreamSamples();
	((Streaming*)arg)->running = false;
	((Streaming*)arg)->OnStreamingEnded();
	return 0;
}

void Streaming::StreamSamples() {
	bool sts = true;

	/* Create a PXCSenseManager instance */
	sm = PXCSenseManager::CreateInstance();
	if (!sm) {
		UpdateStatus(L"Init Failed");
		return;
	}

	/* Optional: Set file name for playback or recording */
	PXCCaptureManager *cm = sm->QueryCaptureManager();
	if (record && file[0]) cm->SetFileName(file, true);
	if (playback && file[0]) cm->SetFileName(file, false);

	/* Optional: Set device source */
	if (!playback) cm->FilterByDeviceInfo(&dinfo);

	while (true) {

		/* Initialization */
		UpdateStatus(L"Init Started");
		if (sm->Init(this)<PXC_STATUS_NO_ERROR) {
			UpdateStatus(L"Init Failed");
			sts = false;
			break;
		}

		/* Optional but recommended: Reset all properties */
		PXCCapture::Device *device = cm->QueryDevice();
		device->ResetProperties(PXCCapture::STREAM_TYPE_ANY);

		/* Optional: Set mirror mode */
		device->SetMirrorMode(mirror ? PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL : PXCCapture::Device::MirrorMode::MIRROR_MODE_DISABLED);

		UpdateStatus(L"Streaming...");
		pxcStatus sts2 = PXC_STATUS_NO_ERROR;
		for (int nframes = 0; !stop; nframes++) {
			/* Wait until a frame is ready, synchronized or asynchronously */
			sts2 = sm->AcquireFrame(synced);
			if (sts2<PXC_STATUS_NO_ERROR && sts2 != PXC_STATUS_DEVICE_LOST) break;

			/* If device is not lost */
			if (sts2 >= PXC_STATUS_NO_ERROR) {
				/* Retrieve the sample */
				PXCCapture::Sample *sample = sm->QuerySample();

				/* Display Main & PIP pictures */
				bool pip_update = false;
				if (pip) {
					PXCImage *image = (*sample)[pip];
					if (image) {
						DisplayPIPImage(image);
						pip_update = true;
					}
				}

				PXCImage::ImageInfo info = {};
				if (main)  {
					PXCImage* image = (*sample)[main];
					if (image) {
						DisplayMainImage(image);
						pip_update = false;  // pip is updated here as well.
						info = image->QueryInfo();
					}
				}

				if (pip_update) DisplayMainImage(0);
				if (info.width > 0) Tick(&info);
			}

			/* Optional: Reset the mirro mode */
			PXCCapture::Device::MirrorMode mirror2 = mirror ? PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL : PXCCapture::Device::MirrorMode::MIRROR_MODE_DISABLED;
			if (device->QueryMirrorMode() != mirror2)
				device->SetMirrorMode(mirror2);

			/* Resume next frame processing */
			sm->ReleaseFrame();
		}

		/* Optional: Handle device configuration change */
		if (sts2 == PXC_STATUS_STREAM_CONFIG_CHANGED) {
			/* profile was changed for adaptive stream */
			UpdateStatus(L"Adaptive");
			/* restart SenseManager */
			sm->Close();
			continue;
		}
		break;
	}

	/* Clean Up */
	if (sts) UpdateStatus(L"Stopped");
	sm->Release();
}