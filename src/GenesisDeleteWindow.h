/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISDELETEWINDOW_H_
#define _GENESISDELETEWINDOW_H_

#include "GenesisCustomListView.h"

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <StringView.h>
#include <StatusBar.h>
#include <OS.h>

const uint32 BUTTON_MSG_ABORT		= 'BMAB';
const uint32 BUTTON_MSG_PAUSE		= 'BMPA';

int32 DeleteThreadFunc(void *data);

class GenesisDeleteWindow : public BWindow
{
	public:
		GenesisDeleteWindow(CustomListView *list, BLooper* looper,BWindow *mainwindow);
		~GenesisDeleteWindow();

		void RemoveParentSelection();
		void Go(void);
		void DeleteDirectory(const char *dirname);
		bool UniversalDelete(const char *filename);
		void DeleteError(const char *filename);
		void FolderNotEmpty(const char *dirname);
		int32 GetFirstSelection(void);

		CustomListView	*m_CustomListView;
		BWindow			*m_Window;
		BView 			*m_View;
		BButton 		*m_AbortButton;
		BButton 		*m_PauseButton;
		BTextControl	*m_DirName;
		BLooper			*m_Looper;
		BStatusBar		*m_ProgressBar;
//		BStringView		*m_Label;
		
		int32			m_Selection;
		
		int 			m_FileCount;
		bool			m_Paused;
		bool			m_SkipAllError;
		bool			m_DeleteAllNotEmpty;
		thread_id 		m_DeleteThread;

		virtual void	MessageReceived(BMessage* message);
};

#endif
