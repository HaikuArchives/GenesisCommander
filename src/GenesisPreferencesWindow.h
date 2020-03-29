/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019-2020, Ondrej Čerman
 */

#ifndef _GENESISPREFERENCESWINDOW_H_
#define _GENESISPREFERENCESWINDOW_H_

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <String.h>
#include <Box.h>
#include <CheckBox.h>
#include <Menu.h>

const uint32 BUTTON_MSG_APPLY				= 'BPAP';
const uint32 BUTTON_MSG_CANCEL				= 'BPCA';
const uint32 PREFERENCES_CHANGED			= 'PPCH';
const uint32 BUTTON_MSG_SET_CURR_PATH_L		= 'BPPL';
const uint32 BUTTON_MSG_SET_CURR_PATH_R		= 'BPPR';
const uint32 MENU_MSG_SET_EDITOR			= 'MSSE';

class GenesisPreferencesWindow : public BWindow
{
	public:
		GenesisPreferencesWindow(BLooper* looper, BWindow *mainwindow);
		~GenesisPreferencesWindow();


		BLooper			*m_Looper;

		BButton			*m_ApplyButton;
		BCheckBox		*m_ShowFunctionKeys;
		BCheckBox		*m_ShowCommandLine;
		BCheckBox		*m_SymlinkedPaths;
		BCheckBox		*m_AskOnExit;
		BTextControl	*m_LeftPanelPath;
		BTextControl	*m_RightPanelPath;
		BTextControl	*m_TerminalWindowTitle;
		BTextControl	*m_EditorApp;
		BMenu			*m_EditorsMenu;

		virtual void	MessageReceived(BMessage* message);

	private:
		void			FillEditors(void);
		void			ReloadSettings(void);
		void 			ApplySettings(void);
};

#endif
