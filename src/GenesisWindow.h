/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISWINDOW_H_
#define _GENESISWINDOW_H_

//#include "GenesisPanelView.h"
#include <View.h>
#include <Window.h>
#include <ListView.h>
#include <StringView.h>
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>
#include <TextControl.h>
#include <MessageFilter.h>
#include <String.h>

#define __VER__ "0.43"

#define MAINWINDOW GenesisWindow::m_MainWindow

const uint32 MENU_EXIT					= 'MExt';
const uint32 MENU_GETINFO				= 'MInf';
const uint32 MENU_GETNODEINFO			= 'MNnf';
const uint32 MENU_PREFERENCES			= 'MPre';
const uint32 MENU_SELECT_ALL			= 'MSal';
const uint32 MENU_DESELECT_ALL			= 'MDal';
const uint32 MENU_INVERT				= 'MInv';
const uint32 MENU_ADD_FOLDERS			= 'MAfo';
const uint32 MENU_ADD_FILES				= 'MAfi';
const uint32 MENU_ADD_SYMLINKS			= 'MAsl';
const uint32 MENU_SELECT_GROUP 			= 'MSgp';
const uint32 MENU_DESELECT_GROUP 		= 'MDgp';
const uint32 MENU_SEEK					= 'MSek';
const uint32 MENU_RESTART_INPUTSERVER 	= 'MRis';
const uint32 MENU_RESTART_MEDIASERVER 	= 'MRms';
const uint32 MENU_RESTART_NETWORKSERVER = 'MRns';
const uint32 MENU_TERMINAL 				= 'MTRM';
const uint32 MENU_RELOAD 				= 'MRLD';
const uint32 MENU_SWAP_PANELS			= 'MSWP';
const uint32 MENU_TARGET_SOURCE			= 'MTES';

const uint32 MENU_COMMANDS_VIEW			= 'MCVW';
const uint32 MENU_COMMANDS_EDIT			= 'MCED';
const uint32 MENU_COMMANDS_EDITNEW		= 'MCEN';
const uint32 MENU_COMMANDS_COPY			= 'MCCP';
const uint32 MENU_COMMANDS_MOVE			= 'MCMO';
const uint32 MENU_COMMANDS_RENAME		= 'MCRE';
const uint32 MENU_COMMANDS_MKDIR		= 'MCMK';
const uint32 MENU_COMMANDS_DELETE		= 'MCDE';
const uint32 MENU_COMMANDS_CREATE_SYMLINK = 'MCCS';
const uint32 MENU_ABOUT					= 'MABO';

const uint32 BUTTON_MSG_F3	= 'BF3';
const uint32 BUTTON_MSG_F4	= 'BF4';
const uint32 BUTTON_MSG_F5	= 'BF5';
const uint32 BUTTON_MSG_F6	= 'BF6';
const uint32 BUTTON_MSG_F7	= 'BF7';
const uint32 BUTTON_MSG_F8	= 'BF8';
const uint32 BUTTON_MSG_F9	= 'BF9';
const uint32 BUTTON_MSG_F10	= 'BF10';

const uint32 MSG_FILELISTVIEW_SELECTION = 'FVSE';
const uint32 MSG_UPDATEPANEL_SELECTION  = 'UPPS';
const uint32 MSG_UPDATECOMMANDLINE_PATH = 'UCLP';
const uint32 MSG_ACTIVATE_COMMAND_LINE  = 'MACL';
const uint32 MSG_COMMAND_LINE_ESC		= 'CESC';
const uint32 MSG_COMMAND_LINE_ENTER		= 'CENT';

class BButton;
class Language;
class Settings;
class PanelView;
class CommandLine;

class GenesisWindow : public BWindow
{
	public:
		GenesisWindow();
		~GenesisWindow();

		enum MousePointer
		{
			CR_DEFAULT,
			CR_HOURGLASS,
		};
						
		static GenesisWindow *m_MainWindow;
						
		virtual bool	QuitRequested();
		virtual void	MessageReceived(BMessage* message);
		virtual void	FrameResized(float width, float height);
		
		PanelView		*GetActivePanel(void);
		PanelView		*GetInactivePanel(void);
		void 			UpdatePanels(void);
		void 			UpdateCommandLinePath(void);

		void 			SetMousePointer(MousePointer mp);

		BView			*m_MainView;
		BMenuBar		*m_MenuBar;

		PanelView		*m_LeftPanel;
		PanelView		*m_RightPanel;
		
		BButton			*m_Button_F3;
		BButton			*m_Button_F4;
		BButton			*m_Button_F5;
		BButton			*m_Button_F6;
		BButton			*m_Button_F7;
		BButton			*m_Button_F8;
		BButton			*m_Button_F9;
		BButton			*m_Button_F10;
		
		CommandLine		*m_CommandLine;
		
		BString			m_AppName;
		BString			m_AppPath;
		
		Settings		*m_Settings;
		Language		*m_Language;
	private:
		void SetMousePointerShape(int n);
	
		MousePointer	m_MousePointer;
		bool			m_MousePointerChanged;
};

class EscapeFilter : public BMessageFilter
{
	public:
		EscapeFilter(BWindow* window, BMessage* msg);
		~EscapeFilter();

	BWindow		*m_TargetWindow;
	BMessage	*m_MessageToSend;
		
	virtual filter_result Filter(BMessage* msg, BHandler** target);
};

class KeyboardFilter : public BMessageFilter
{
	public:
		KeyboardFilter(BWindow* window, BMessage* msg);
		~KeyboardFilter();

	BWindow		*m_TargetWindow;
	BMessage	*m_MessageToSend;
		
	virtual filter_result Filter(BMessage* msg, BHandler** target);
};

class SeekFilter : public BMessageFilter
{
	public:
		SeekFilter(BWindow* window, PanelView *panel, BMessage* msg);
		~SeekFilter();

	BWindow		*m_TargetWindow;
	BMessage	*m_MessageToSend;
	PanelView	*m_PV;
		
	virtual filter_result Filter(BMessage* msg, BHandler** target);
};

#endif
