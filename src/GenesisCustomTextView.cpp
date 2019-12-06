/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
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
	BRect b = Bounds();
	FrameResized(b.Width(), b.Height());
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
	BRect textrect = Bounds();
	textrect.InsetBy(3.0,3.0);
	SetTextRect(textrect);
	BTextView::FrameResized(width, height);
}

