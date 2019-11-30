/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISCUSTOMTEXTVIEW_H_
#define _GENESISCUSTOMTEXTVIEW_H_

#include <View.h>
#include <TextControl.h>
#include <StringView.h>
#include <ListView.h>

class CustomTextView : public BTextView
{
	public:
		CustomTextView(BRect rect, char *name);
		~CustomTextView();

		virtual void KeyDown(const char *bytes, int32 numBytes);
		virtual void FrameResized(float width, float height);
};

#endif
