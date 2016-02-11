/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2010-2015 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include "pip_render.h"

PIPRender::PIPRender() :D2D1Render(), pip_scale(0), pip_bitmap(0) {
}

PIPRender::~PIPRender() {
	if (pip_bitmap) pip_bitmap->Release();
}

void PIPRender::UpdatePIPPanel(PXCImage* image) {
	if (!image) return;

	EnterCriticalSection(&cs);
	PXCImage::ImageData data = {};
	PXCImage::Rotation rotation = image->QueryRotation();
	pxcStatus sts = image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, rotation, PXCImage::OPTION_ANY, &data);
	if (sts >= PXC_STATUS_NO_ERROR) {
		D2D1_BITMAP_PROPERTIES properties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
		PXCImage::ImageInfo iinfo = image->QueryInfo();
		if (rotation == PXCImage::ROTATION_90_DEGREE || rotation == PXCImage::ROTATION_270_DEGREE)
		{
			int w = iinfo.width;
			iinfo.width = iinfo.height;
			iinfo.height = w;
		}

		HRESULT hr = E_FAIL;
		if (pip_bitmap) {
			D2D1_SIZE_U bsize = pip_bitmap->GetPixelSize();
			if (bsize.width == iinfo.width && bsize.height == iinfo.height) {
				hr = pip_bitmap->CopyFromMemory((const D2D1_RECT_U*)&D2D1::RectU(0, 0, iinfo.width, iinfo.height), data.planes[0], data.pitches[0]);
			}
		}

		if (FAILED(hr)) {
			if (pip_bitmap) pip_bitmap->Release(), pip_bitmap = 0;
			hr = context2->CreateBitmap(D2D1::SizeU(iinfo.width, iinfo.height), data.planes[0], data.pitches[0], properties, &pip_bitmap);
		}

		image->ReleaseAccess(&data);
	}
	LeaveCriticalSection(&cs);
}

void PIPRender::UpdatePanelEx(ID2D1Bitmap* bitmap) {
	D2D1Render::UpdatePanelEx(bitmap);

	if (pip_bitmap && pip_scale>0) {
		D2D1_SIZE_F bsize = pip_bitmap->GetSize();
		D2D1_SIZE_F psize = context2->GetSize();

		float pwidth = (float)(int)(psize.width / pip_scale);
		float pheight = (float)(int)(psize.height / pip_scale);

		float sx = pwidth / bsize.width;
		float sy = pheight / bsize.height;
		float sxy = min(sx, sy);
		sx = sxy * bsize.width;
		sy = sxy * bsize.height;

		context2->DrawBitmap(pip_bitmap, D2D1::RectF(psize.width - sx, psize.height - sy, psize.width, psize.height));
	}
}