/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISVIEWWINDOW_H_
#define _GENESISVIEWWINDOW_H_

#include "GenesisCustomTextView.h"
#include <Window.h>
#include <Bitmap.h>
#include <Volume.h>
#include <Picture.h>
#include <TextView.h>
#include <String.h>
#include <ScrollView.h>
#include <Menu.h>

const uint32 VIEWMENU_FILE_CLOSE		= 'VFCL';
const uint32 VIEWMENU_FILE_COPY			= 'VFCP';
const uint32 VIEWMENU_FILE_SELECTALL	= 'VFSE';
const uint32 VIEWMENU_FILE_WORDWRAP		= 'VFWW';

class GenesisViewWindow : public BWindow
{
	public:
		GenesisViewWindow(const char* filename, BWindow *mainwindow = NULL);
		~GenesisViewWindow();

		BString			m_FileName;
		BView			*m_View;
		CustomTextView	*m_TextView;
		BScrollView 	*m_ScrollView;
		BMenuBar		*m_MenuBar;

		BMenuItem		*m_MI_WordWrap;

		virtual void	MessageReceived(BMessage* message);
};

#endif
