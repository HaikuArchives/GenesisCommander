/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019-2020, Ondrej Čerman
 */

#ifndef _GENESISCOPYWINDOW_H_
#define _GENESISCOPYWINDOW_H_

#include "GenesisPanelView.h"

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <String.h>
#include <StatusBar.h>
#include <OS.h>
#include <MessageFilter.h>
#include <CheckBox.h>

const uint32 BUTTON_MSG_COPY		= 'BMCP';
const uint32 BUTTON_MSG_CANCELCOPY	= 'BMCC';
const uint32 COPYNAME_CHANGED		= 'CNCH';
const uint32 BUTTON_MSG_PAUSECOPY	= 'BPAU';
const uint32 BUTTON_MSG_ABORTCOPY	= 'BABT';
//const uint32 COPY_MORE				= 'CMRE';

class GenesisCopyWindow : public BWindow
{
	public:
		GenesisCopyWindow(CustomListView *list, PanelView *destpanel, const char *destination,BLooper* looper, BWindow *mainwindow);
		~GenesisCopyWindow();

		CustomListView	*m_CustomListView;
		BWindow			*m_Window;
		BView 			*m_View;
		BButton 		*m_CopyButton;
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
//		BCheckBox		*m_MoreCB;

		int				m_FileCount;
		int32			m_Selection;

		thread_id		m_CopyThread;

		virtual void	MessageReceived(BMessage* message);

		bool			m_SingleCopy;
		bool			m_PossiblyMultipleFiles;
		bool			m_Paused;
		bool			m_SkipAllCopyError;
		bool 			m_OverwriteAll;
		bool 			m_CopyAll;
		bool			m_SkipSymLinkCreationError;

		void			Go(void);

		void RemoveParentSelection();
		void PrepareCopy(void);
		void Copy(const char *filename, const char *destination, const char *destfilename = NULL);
		bool CopyFile(const char *filename, const char *destination, const char *destfilename = NULL);
		void CopyDirectory(const char *dirname, const char *destination, const char *destdirname = NULL);
		bool CopyLink(const char *linkname, const char *destination, const char *destfilename = NULL);
		bool CopyAttr(const char *srcfilename, const char *dstfilename);
		int32 GetFirstSelection(void);
		bool IsDirReadOnly(const char *destination);
		bool IsRecursiveCopy(const char *source, const char *destination);

		ALERT_SKIP_OPTS CopySkipAlert(const char* text);
		ALERT_OVERWR_OPTS CopyOverwriteAlert(const char* text);
};

#endif
