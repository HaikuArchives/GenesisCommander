/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#ifndef _GENESISMAKEFILEWINDOW_H_
#define _GENESISMAKEFILEWINDOW_H_

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <String.h>

const uint32 BUTTON_MSG_CREATE_FILE	= 'BMKF';
const uint32 BUTTON_MSG_CANCEL		= 'BMKC';
const uint32 FILENAME_CHANGED		= 'FNCH';

class GenesisMakeFileWindow : public BWindow
{
	public:
		GenesisMakeFileWindow(const char* dirpath, BLooper* looper,BWindow *mainwindow = NULL, bool directory = true, bool edit = false);
		~GenesisMakeFileWindow();

		bool CreateFolder(const char *dirpath, const char *dirname);
		bool CreateFile(const char *path, const char *filename);

		BView 			*m_View;
		BButton 		*m_OkButton;
		BTextControl	*m_FileName;
		BLooper			*m_Looper;
		BString			m_DirPath;
		bool			m_MkDirMode;
		bool			m_EditAfter;

		virtual void	MessageReceived(BMessage* message);
};

class CustomTextControl : public BTextControl
{
	public:
		CustomTextControl(BRect rect, const char *name, const char *label, const char *text);
		~CustomTextControl();

//		virtual void KeyUp(const char *bytes, int32 numBytes);
		virtual void	MessageReceived(BMessage* message);

//		void SetESCMessage(BMessage *msg);
};


#endif
