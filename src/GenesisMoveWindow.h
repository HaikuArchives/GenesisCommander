/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019-2020, Ondrej Čerman
 */

#ifndef _GENESISMOVEWINDOW_H_
#define _GENESISMOVEWINDOW_H_

#include "GenesisPanelView.h"

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <String.h>
#include <StatusBar.h>
#include <OS.h>
#include <MessageFilter.h>

const uint32 BUTTON_MSG_MOVE		= 'BMMO';
const uint32 BUTTON_MSG_CANCELMOVE	= 'BMCM';
const uint32 MOVENAME_CHANGED		= 'MNCH';
const uint32 BUTTON_MSG_PAUSEMOVE	= 'BPAM';
const uint32 BUTTON_MSG_ABORTMOVE	= 'BABM';

class GenesisMoveWindow : public BWindow
{
	public:
		GenesisMoveWindow(CustomListView *list, PanelView *destpanel, const char *destination,BLooper* looper, BWindow *mainwindow);
		~GenesisMoveWindow();

		CustomListView	*m_CustomListView;
		BWindow			*m_Window;
		BView 			*m_View;
		BButton 		*m_MoveButton;
		BButton			*m_CancelButton;
		BButton			*m_AbortButton;
		BButton			*m_PauseButton;
		BTextControl	*m_DirName;
		BTextControl	*m_FileAsName;
		BStringView		*m_Label;
		BLooper			*m_Looper;
		BString			m_DestPath;	// Destination path
		BString			m_DestFileName;
		BStatusBar		*m_ProgressBar;
		BStatusBar		*m_FileBar;
		PanelView		*m_DestPanel;

		int				m_FileCount;
		int				m_TopFilesLeft;
		int				m_CurrentLevel;
		int32			m_Selection;

		thread_id		m_MoveThread;

		virtual void	MessageReceived(BMessage* message);

		bool			m_SingleMove;
		bool			m_PossiblyMultipleFiles;
		bool			m_Paused;
		bool			m_SkipAllMoveError;
		bool 			m_OverwriteAll;
		bool 			m_MoveAll;
		bool			m_SkipSymLinkCreationError;
		bool			m_SkipAllMissing;		// move

		void			Go(void);

		void RemoveParentSelection();
		void PrepareMove(void);
		bool Move(const char *filename, const char *destination, const char *destfilename = NULL);
		bool CopyAndDelete(BEntry *sourceentry, BEntry *dstentry, BDirectory *dstdir);
		int32 GetFirstSelection(void);
		bool IsDirReadOnly(const char *destination);
		bool IsRecursiveMove(const char *source, const char *destination);

		ALERT_SKIP_OPTS MoveSkipAlert(const char* text);
		ALERT_OVERWR_OPTS MoveOverwriteAlert(const char* text);
};

#endif
