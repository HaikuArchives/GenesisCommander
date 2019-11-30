/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISMAKEDIRWINDOW_H_
#define _GENESISMAKEDIRWINDOW_H_

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <String.h>

const uint32 BUTTON_MSG_CREATE_DIR	= 'BMKD';
const uint32 BUTTON_MSG_CANCEL		= 'BMKC';
const uint32 DIRNAME_CHANGED		= 'DNCH';

class GenesisMakeDirWindow : public BWindow
{
	public:
		GenesisMakeDirWindow(const char* dirpath, BLooper* looper,BWindow *mainwindow = NULL);
		~GenesisMakeDirWindow();

		bool CreateFolder(const char *dirpath, const char *dirname);

		BView 			*m_View;
		BButton 		*m_OkButton;
		BTextControl	*m_DirName;
		BLooper			*m_Looper;
		BString			m_DirPath;

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
