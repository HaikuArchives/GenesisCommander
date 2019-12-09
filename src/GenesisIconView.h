/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019, Ondrej Čerman
 */

#ifndef _GENESISICONVIEW_H_
#define _GENESISICONVIEW_H_

#include <NodeInfo.h>
#include <Volume.h>
#include <View.h>

class IconView : public BView
{
public:
					IconView(BRect rect, const char *name);
					~IconView(void);
	virtual void	Draw(BRect rect);
	bool			SetIcon(BNodeInfo *nodeinfo);
	bool			SetIcon(BVolume *volume);

private:
	BBitmap			*m_Icon;
};

#endif
