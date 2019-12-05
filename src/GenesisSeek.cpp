/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisSeek.h"
#include <Rect.h>
#include <Beep.h>
#include <Messenger.h>
#include <Handler.h>

////////////////////////////////////////////////////////////////////////
SeekControl::SeekControl(BRect rect, const char *name, BMessage* msg)
	   	   : BTextControl( rect, name , "Seek for", "", msg, B_FOLLOW_BOTTOM | B_FOLLOW_LEFT , B_WILL_DRAW )
////////////////////////////////////////////////////////////////////////
{
	BFont font;
	GetFont(&font);
	SetDivider(font.StringWidth("Seek for")+4);

	TextView()->DisallowChar(B_FUNCTION_KEY);
	TextView()->DisallowChar(B_INSERT);
	TextView()->DisallowChar(B_ESCAPE);
}

////////////////////////////////////////////////////////////////////////
SeekControl::~SeekControl()
////////////////////////////////////////////////////////////////////////
{

}
