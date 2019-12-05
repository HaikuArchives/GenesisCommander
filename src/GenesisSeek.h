/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISSEEKLINE_H_
#define _GENESISSEEKLINE_H_

#include <TextControl.h>
#include <View.h>
#include <String.h>

const uint32 SEEK_ESC		= 'SESC';

class SeekControl : public BTextControl
{
	public:
		SeekControl(BRect rect, const char *name, BMessage* msg);
		~SeekControl();
};

#endif
