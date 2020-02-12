/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#ifndef _GENESISGETINFOWINDOW_H_
#define _GENESISGETINFOWINDOW_H_

#include "GenesisCustomListItem.h"
#include "GenesisIconView.h"
#include <Box.h>
#include <Window.h>
#include <Bitmap.h>
#include <Volume.h>
#include <Picture.h>
#include <StringList.h>

enum {
	ET_DIRECTORY,
	ET_SYMLINK,
	ET_FILE,
};

const uint32 BUTTON_MSG_OK	= 'BMOK';

class GenesisGetInfoWindow : public BWindow
{
	public:
		GenesisGetInfoWindow(const char *dir, BStringList *files, BWindow *mainwindow = NULL);
		~GenesisGetInfoWindow();

		void ExamineDirectory(const char* filename);
		void ExamineSymLink(const char* filename);
		void ExamineFile(const char* filename);
		void ExamineMultipleFiles(const char *dir, const BStringList *filesList);

		virtual void	MessageReceived(BMessage* message);

		IconView *m_IconView;
		BView *m_View;
		BBox *m_IconBox;
};

class PieView : public BView
{
	public:
		PieView(BRect frame, const char *name, off_t capacity, off_t free);
		~PieView();

		virtual void Draw(BRect r);

		off_t m_Capacity, m_Free;
};

class GenesisGetDiskInfoWindow : public BWindow
{
	public:
		GenesisGetDiskInfoWindow(CustomListItem *item, BWindow *mainwindow = NULL);
		~GenesisGetDiskInfoWindow();

		void ExamineDevice(BVolume *v);

		virtual void	MessageReceived(BMessage* message);

		IconView *m_IconView;
		BView *m_View;

		PieView *m_PieView;
};

#endif
