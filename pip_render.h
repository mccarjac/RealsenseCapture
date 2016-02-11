/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2010-2015 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include "d2d1_render.h"

/* Rendering Class with Picture in Picture Window */
class PIPRender : public D2D1Render {
public:

	PIPRender();
	virtual ~PIPRender();

	void SetPIPScale(int scale) { pip_scale = scale; }
	int  GetPIPScale() { return pip_scale; }

	void UpdatePIPPanel(PXCImage* image);

protected:

	int				pip_scale;
	ID2D1Bitmap*	pip_bitmap;

	virtual void UpdatePanelEx(ID2D1Bitmap *bitmap);
};