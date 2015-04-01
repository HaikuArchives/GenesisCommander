#ifndef _GENESISSEEKLINE_H_
#define _GENESISSEEKLINE_H_

#include <TextControl.h>
#include <View.h>
#include <String.h>

const uint32 SEEK_ESC		= 'SESC';

class SeekControl : public BTextControl
{
	public:
		SeekControl(BRect rect, char *name, BMessage* msg);
		~SeekControl();
};

#endif