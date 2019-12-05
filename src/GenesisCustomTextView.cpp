/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisCustomTextView.h"
#include "GenesisViewWindow.h"
#include <Beep.h>
#include <TextView.h>

////////////////////////////////////////////////////////////////////////
CustomTextView::CustomTextView(BRect rect, const char *name) :
	BTextView(rect, name, rect, B_FOLLOW_ALL , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
//	BRect r;

//	r = Bounds();
//	r.right -= 8;
//	r.bottom -= 16;
//	SetTextRect(r);
}

////////////////////////////////////////////////////////////////////////
CustomTextView::~CustomTextView()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void CustomTextView::KeyDown(const char *bytes, int32 numBytes)
////////////////////////////////////////////////////////////////////////
{
	int process = true;

	for (int i=0;i<numBytes;i++)
	{
		switch (bytes[i])
		{
			case B_ESCAPE:
				((GenesisViewWindow *)Window())->Close();
				break;
			case B_HOME:
				ScrollToOffset(0);
				process = false;
				break;
			case B_END:
				ScrollToOffset(TextLength());
				process = false;
				break;
		}
	}
	
	if (process)
		BTextView::KeyDown(bytes,numBytes);
}

////////////////////////////////////////////////////////////////////////
void CustomTextView::FrameResized(float width, float height)
////////////////////////////////////////////////////////////////////////
{
//	BRect r;

	BTextView::FrameResized(width, height);

//	r = Bounds();
//	SetTextRect(r);
}

