/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2015 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include "pxcsensemanager.h"
#include "timer.h"

class Streaming : public PXCSenseManager::Handler, public Timer {
public:

	Streaming();
	virtual ~Streaming();

	void SetAdaptive(bool adaptive) { this->adaptive = adaptive; }
	void SetSynced(bool synced) { this->synced = synced; }
	void SetLive() { record = playback = false; }
	void SetRecord(pxcCHAR *file) { record = true; playback = false; this->file = file; }
	void SetPlayback(pxcCHAR *file) { record = false; playback = true; this->file = file; }
	void SetDevice(PXCCapture::DeviceInfo dinfo) { this->dinfo = dinfo; }
	void SetStreams(PXCCapture::Device::StreamProfileSet profiles) { this->profiles = profiles; }
	void SetMirror(bool mirror) { this->mirror = mirror; }
	bool GetMirror() { return mirror; }
	void SetMain(PXCCapture::StreamType stype) { main = stype; }
	void SetPIP(PXCCapture::StreamType stype) { pip = stype; }
	PXCCapture::StreamType GetMain() { return main; }

	void Start();
	void Stop();

	bool IsRunning() { return running; }

protected:

	PXCSenseManager* sm;
	PXCCapture::DeviceInfo dinfo;
	PXCCapture::Device::StreamProfileSet profiles;
	PXCCapture::StreamType main;
	PXCCapture::StreamType pip;
	bool adaptive;
	bool synced;
	bool record;
	bool playback;
	pxcCHAR *file;
	bool mirror;
	volatile bool running;
	volatile bool stop;
	HANDLE thread;

	static DWORD WINAPI ThreadProc(LPVOID arg);
	void StreamSamples();

	virtual pxcStatus PXCAPI OnConnect(PXCCapture::Device *device, pxcBool connected);
	virtual void UpdateStatus(pxcCHAR* /*line*/) {};
	virtual void DisplayMainImage(PXCImage* /*bitmap*/) {};
	virtual void DisplayPIPImage(PXCImage* /*bitmap*/) {};
	virtual void OnStreamingEnded() {};
};