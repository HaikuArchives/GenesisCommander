/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#include "GenesisWindow.h"
#include "GenesisCustomListView.h"
#include "GenesisSeek.h"
#include "GenesisPanelView.h"
#include "GenesisPreferencesWindow.h"
#include "Language.h"
#include "Settings.h"
#include "GenesisApp.h"
#include <Application.h>
#include <stdio.h>
#include <stdlib.h>
#include <Box.h>
#include <Screen.h>
#include <Beep.h>
#include <Alert.h>
#include <Button.h>
#include <Cursor.h>

GenesisWindow *GenesisWindow::m_MainWindow = NULL;

const BRect kWindowFrame (100,100,700,584);

////////////////////////////////////////////////////////////////////////
GenesisWindow::GenesisWindow() :
	BWindow(kWindowFrame, "Genesis Commander", B_TITLED_WINDOW, B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	m_MainWindow = this;
	m_MousePointerChanged = false;
	m_MousePointer = CR_DEFAULT;

	MoveTo(SETTINGS->GetWindowLeft(), SETTINGS->GetWindowTop());
	ResizeTo(SETTINGS->GetWindowWidth(), SETTINGS->GetWindowHeight());

	BMenu *menu;
	BString title;
	BMenuItem *menuitem;
	
	// Language singleton letrehozasa...
	m_Language = new Language(SETTINGS->GetLanguage());

	title.SetTo(LANG("WINDOW_TITLE"));
	title.ReplaceAll("<VER>",__VER__);
	SetTitle(title.String());

	// Main view
	m_MainView = new BView(Bounds(), "mainview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_MainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_MainView);

	// Menu
	m_MenuBar = new BMenuBar(Bounds(),"mainmenu");

	// File menu
	menu = new BMenu(LANGS("MENU_FILE"));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_GETINFO") , new BMessage(MENU_GETINFO), 'I'));
//	menu->AddItem(new BMenuItem("Get node info..." , new BMessage(MENU_GETNODEINFO), 0));
	menuitem = new BMenuItem(LANGS("SUBMENU_GETNODEINFO") , new BMessage(MENU_GETNODEINFO), 0);
	menuitem->SetEnabled(false);
	menu->AddItem(menuitem);

	menu->AddSeparatorItem();

	menu->AddItem(new BMenuItem(LANGS("SUBMENU_PREFERENCES") , new BMessage(MENU_PREFERENCES), 'P'));

	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_QUIT") , new BMessage(MENU_EXIT), 'Q'));	
	m_MenuBar->AddItem(menu);

	menu = new BMenu(LANGS("MENU_SELECTION"));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_SELECTALL") , new BMessage(MENU_SELECT_ALL), '+'));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_DESELECTALL") , new BMessage(MENU_DESELECT_ALL), '-'));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_INVERTSELECTION") , new BMessage(MENU_INVERT), '*'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_ADDALLFOLDERS") , new BMessage(MENU_ADD_FOLDERS), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_ADDALLFILES") , new BMessage(MENU_ADD_FILES), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_ADDALLSYMLINKS") , new BMessage(MENU_ADD_SYMLINKS), 0));
	menu->AddSeparatorItem();
//	menu->AddItem(new BMenuItem("Select group..." , new BMessage(MENU_SELECT_GROUP), 0));
	menuitem = new BMenuItem(LANGS("SUBMENU_SELECTGROUP") , new BMessage(MENU_SELECT_GROUP), 0);
	menuitem->SetEnabled(false);
	menu->AddItem(menuitem);
	
//	menu->AddItem(new BMenuItem("Deselect group..." , new BMessage(MENU_DESELECT_GROUP), 0));
	menuitem = new BMenuItem(LANGS("SUBMENU_DESELECTGROUP") , new BMessage(MENU_DESELECT_GROUP), 0);
	menuitem->SetEnabled(false);
	menu->AddItem(menuitem);

	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_SEEKINLIST") , new BMessage(MENU_SEEK), 'S'));
	m_MenuBar->AddItem(menu);

	menu = new BMenu(LANGS("MENU_COMMANDS"));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_VIEW"),		new BMessage(MENU_COMMANDS_VIEW), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_EDIT"),		new BMessage(MENU_COMMANDS_EDIT), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_EDITNEW"),	new BMessage(MENU_COMMANDS_EDITNEW), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_COPY"),	 	new BMessage(MENU_COMMANDS_COPY), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_MOVE"),		new BMessage(MENU_COMMANDS_MOVE), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_RENAME"),	new BMessage(MENU_COMMANDS_RENAME), 'N'));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_MAKEDIR"),	new BMessage(MENU_COMMANDS_MKDIR), 0));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_DELETE"),	new BMessage(MENU_COMMANDS_DELETE), 0));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_CREATELINK") , new BMessage(MENU_COMMANDS_CREATE_SYMLINK), 'L'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_TERMINAL"), new BMessage(MENU_TERMINAL), 'T', B_OPTION_KEY));
	m_MenuBar->AddItem(menu);

	menu = new BMenu(LANGS("MENU_PANELS"));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_RELOAD"),			new BMessage(MENU_RELOAD), 'R'));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_SWAP"),				new BMessage(MENU_SWAP_PANELS), 'U'));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_TARGET_EQ_SOURCE"),	new BMessage(MENU_TARGET_SOURCE), 0));
	m_MenuBar->AddItem(menu);

	menu = new BMenu(LANGS("MENU_HELP"));
	menu->AddItem(new BMenuItem(LANGS("SUBMENU_ABOUT") , new BMessage(MENU_ABOUT), 0));
	m_MenuBar->AddItem(menu);

	m_MainView->AddChild(m_MenuBar);

	// Buttons
	m_FuncKeysVisible = true;
	m_Button_F3 = new BButton(BRect(10,10,40,40),"view",LANGS("BUTTON_F3"),new BMessage(BUTTON_MSG_F3),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F3);
	m_Button_F4 = new BButton(BRect(10,10,40,40),"edit",LANGS("BUTTON_F4"),new BMessage(BUTTON_MSG_F4),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F4);
	m_Button_F5 = new BButton(BRect(10,10,40,40),"copy",LANGS("BUTTON_F5"),new BMessage(BUTTON_MSG_F5),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F5);
	m_Button_F6 = new BButton(BRect(10,10,40,40),"move",LANGS("BUTTON_F6"),new BMessage(BUTTON_MSG_F6),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F6);
	m_Button_F7 = new BButton(BRect(10,10,40,40),"mkdir",LANGS("BUTTON_F7"),new BMessage(BUTTON_MSG_F7),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F7);
	m_Button_F8 = new BButton(BRect(10,10,40,40),"delete",LANGS("BUTTON_F8"),new BMessage(BUTTON_MSG_F8),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F8);
	m_Button_F10 = new BButton(BRect(10,10,40,40),"quit",LANGS("BUTTON_F10"),new BMessage(BUTTON_MSG_F10),0,B_WILL_DRAW);
	m_MainView->AddChild(m_Button_F10);

	// Command line...
	m_CommandLine = new CommandLine(BRect(20,20,100,100),"cmdline", NULL);	// new BMessage(CMD_LINE_MSG)
	m_CommandLine->MoveTo(Bounds().left+2,Bounds().bottom-44);
	m_MainView->AddChild(m_CommandLine);
	
	// Left Panel...
	m_LeftPanel = new PanelView(BRect(10,10,100,100),"leftpanel");	
	m_MainView->AddChild(m_LeftPanel);

	// Right Panel...
	m_RightPanel = new PanelView(BRect(10,10,100,100),"rightpanel");
	m_MainView->AddChild(m_RightPanel);

	m_LeftPanel->ChangePath(SETTINGS->GetLeftPanelPath().String());
	m_RightPanel->ChangePath(SETTINGS->GetRightPanelPath().String());

	UpdateUIVisibility(true);
	FrameResized(0,0);
	SetSizeLimits(400,65535,200,65535);

	AddCommonFilter(new KeyboardFilter(this, new BMessage(MSG_COMMAND_LINE_ENTER)));
	AddCommonFilter(new SeekFilter(this, m_LeftPanel, new BMessage(MSG_FILE_SEEK_END)));
	AddCommonFilter(new SeekFilter(this, m_RightPanel, new BMessage(MSG_FILE_SEEK_END)));

	// Let's set the focus on left panel...
	m_LeftPanel->m_CustomListView->MakeFocus(true);
}

////////////////////////////////////////////////////////////////////////
GenesisWindow::~GenesisWindow()
////////////////////////////////////////////////////////////////////////
{
	m_MainWindow = NULL;

	if (m_Language)
		delete m_Language;
}

////////////////////////////////////////////////////////////////////////
bool GenesisWindow::QuitRequested()
////////////////////////////////////////////////////////////////////////
{
	if (SETTINGS->GetAskOnExit())
	{
		BAlert *myAlert = new BAlert("Genesis",LANGS("QUIT"),LANGS("GENERAL_NO"),LANGS("GENERAL_YES"),NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);
		if (myAlert->Go()==1)
		{
			be_app->PostMessage(B_QUIT_REQUESTED);
			return BWindow::QuitRequested();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		be_app->PostMessage(B_QUIT_REQUESTED);
		return BWindow::QuitRequested();
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case MENU_TARGET_SOURCE:
			{
				BString sourcepath;
				sourcepath.SetTo(GetActivePanel()->m_Path.String());
				GetInactivePanel()->ChangePath(sourcepath.String());
			}
			break;
		case MENU_SWAP_PANELS:
			{
				BString leftpath, rightpath;
				leftpath.SetTo(m_LeftPanel->m_Path.String());
				rightpath.SetTo(m_RightPanel->m_Path.String());
				m_LeftPanel->ChangePath(rightpath.String());
				m_RightPanel->ChangePath(leftpath.String());
			}
			break;
		case MSG_PREFERENCES_CHANGED:
			UpdateUIVisibility();
			FrameResized(0,0);
			break;
		case MSG_COMMAND_LINE_ENTER:
			m_CommandLine->Execute();
			if (GetActivePanel())
				GetActivePanel()->m_CustomListView->MakeFocus(true);
			break;
		case MENU_TERMINAL:
			{
				BString tempstring;
				tempstring.SetTo("Terminal -t \"");
				tempstring << SETTINGS->GetTerminalWindow();
				tempstring << "\" &";
				system(tempstring.String());
			}
			break;
		case MENU_SELECT_ALL:
			if (GetActivePanel())
				GetActivePanel()->SelectAll();
			break;
		case MENU_DESELECT_ALL:
			if (GetActivePanel())
				GetActivePanel()->DeselectAll();
			break;
		case MENU_INVERT:
			if (GetActivePanel())
				GetActivePanel()->InvertSelection();
			break;
		case MENU_ADD_FOLDERS:
			if (GetActivePanel())
				GetActivePanel()->AddToSelection(ADD_FOLDERS);
			break;
		case MENU_ADD_FILES:
			if (GetActivePanel())
				GetActivePanel()->AddToSelection(ADD_FILES);
			break;
		case MENU_ADD_SYMLINKS:
			if (GetActivePanel())
				GetActivePanel()->AddToSelection(ADD_SYMLINKS);
			break;
		case MENU_SEEK:
			if (GetActivePanel())
				GetActivePanel()->SeekModeOn();
			break;
		case MSG_UPDATEPANEL_SELECTION:
			UpdatePanels();
			break;
		case MSG_UPDATECOMMANDLINE_PATH:
			UpdateCommandLinePath();
			break;
		case MENU_RELOAD:
			if (GetActivePanel())
				GetActivePanel()->Reload();
			break;
		case MENU_GETINFO:
			if (GetActivePanel())
				GetActivePanel()->GetInfo();
			break;
		case MENU_PREFERENCES:
			GenesisPreferencesWindow *prefwindow;
			prefwindow = new GenesisPreferencesWindow(Looper(), m_MainWindow);
			prefwindow->Show();
			break;
		case MENU_COMMANDS_VIEW:
		case BUTTON_MSG_F3:
			if (GetActivePanel())
				GetActivePanel()->View();
			break;
		case MENU_COMMANDS_EDIT:
		case BUTTON_MSG_F4:
			if (GetActivePanel())
				GetActivePanel()->Edit();
			break;
		case MENU_COMMANDS_EDITNEW:
			if (GetActivePanel())
				GetActivePanel()->MakeFile(true);
			break;
		case MENU_COMMANDS_COPY:
		case BUTTON_MSG_F5:
			if (GetActivePanel())
				GetActivePanel()->Copy();
			break;
		case MENU_COMMANDS_MOVE:
		case BUTTON_MSG_F6:
			if (GetActivePanel())
				GetActivePanel()->Move();
			break;
		case MENU_COMMANDS_MKDIR:
		case BUTTON_MSG_F7:
			if (GetActivePanel())
				GetActivePanel()->MakeDir();
			break;
		case MENU_COMMANDS_DELETE:
		case BUTTON_MSG_F8:
			if (GetActivePanel())
				GetActivePanel()->Delete();
			break;
		case MSG_ACTIVATE_COMMAND_LINE:
			{
				int8 chr;
				if (!m_CommandLine->IsHidden()){
					m_CommandLine->MakeFocus(true);
					if (message->FindInt8("Chr",&chr)==B_OK)
					{
						BString text;
						BTextView *textview = m_CommandLine->TextView();

						text.SetTo(m_CommandLine->Text());
						text << (char)chr;
						textview->Clear();
						textview->Insert(0,text.String(),strlen(text.String()));
					}
				}
			}
			break;
		case MENU_COMMANDS_CREATE_SYMLINK:
			if (GetActivePanel())
				GetActivePanel()->CreateLinkOnDesktop();
			break;
		case MENU_COMMANDS_RENAME:
			if (GetActivePanel())
				GetActivePanel()->Rename();
			break;
		case BUTTON_MSG_F10:
		case MENU_EXIT:
			{
				be_app->PostMessage(B_QUIT_REQUESTED);
				break;
			}
		case MENU_ABOUT:
			{
				BString text;
				text = LANG("ABOUT");
				
				text.ReplaceAll("<VER>", __VER__);
				text.ReplaceAll("<DATE>", __DATE__);
				text.ReplaceAll("<TIME>", __TIME__);
				text.ReplaceAll("|", "\n");
				
				BAlert *AboutAlert = new BAlert(LANGS("SUBMENU_ABOUT"),text.String(),LANGS("GENERAL_OK"),NULL,NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_INFO_ALERT);
				AboutAlert->Go();			
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisWindow::UpdateUIVisibility(bool initial)
////////////////////////////////////////////////////////////////////////
{
	bool prefshowfuncheys = SETTINGS->GetShowFunctionKeys();
	bool prefshowcommandline = SETTINGS->GetShowCommandLine();

	if (m_FuncKeysVisible != prefshowfuncheys)
	{
		if (prefshowfuncheys == true)
		{
			m_Button_F3->Show();
			m_Button_F4->Show();
			m_Button_F5->Show();
			m_Button_F6->Show();
			m_Button_F7->Show();
			m_Button_F8->Show();
			m_Button_F10->Show();
		}
		else
		{
			m_Button_F3->Hide();
			m_Button_F4->Hide();
			m_Button_F5->Hide();
			m_Button_F6->Hide();
			m_Button_F7->Hide();
			m_Button_F8->Hide();
			m_Button_F10->Hide();
		}
		m_FuncKeysVisible = prefshowfuncheys;
	}

	bool commandlinevisible = (initial || !m_CommandLine->IsHidden());
	if (commandlinevisible != prefshowcommandline)
	{
		if (prefshowcommandline == true)
			m_CommandLine->Show();
		else
			m_CommandLine->Hide();
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisWindow::FrameResized(float width, float height)
////////////////////////////////////////////////////////////////////////
{
	BRect r;
	int bottom;
	float buttonwidth,left;

	r = Bounds();
	bottom = (int)(r.bottom);
	left = r.left;

	// Buttons
	if (m_FuncKeysVisible)
	{
		const int funcbtnsheight = 24;
		
		bottom = bottom-funcbtnsheight;
		
		buttonwidth = (r.right-r.left)/7;
		m_Button_F3->MoveTo(0,bottom);
		m_Button_F3->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F3->Invalidate();

		buttonwidth = (r.right-m_Button_F3->Frame().right)/6;
		m_Button_F4->MoveTo(m_Button_F3->Frame().right,bottom);
		m_Button_F4->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F4->Invalidate();

		buttonwidth = (r.right-m_Button_F4->Frame().right)/5;
		m_Button_F5->MoveTo(m_Button_F4->Frame().right,bottom);
		m_Button_F5->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F5->Invalidate();

		buttonwidth = (r.right-m_Button_F5->Frame().right)/4;
		m_Button_F6->MoveTo(m_Button_F5->Frame().right,bottom);
		m_Button_F6->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F6->Invalidate();

		buttonwidth = (r.right-m_Button_F6->Frame().right)/3;
		m_Button_F7->MoveTo(m_Button_F6->Frame().right,bottom);
		m_Button_F7->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F7->Invalidate();

		buttonwidth = (r.right-m_Button_F7->Frame().right)/2;
		m_Button_F8->MoveTo(m_Button_F7->Frame().right,bottom);
		m_Button_F8->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F8->Invalidate();

		buttonwidth = (r.right-m_Button_F8->Frame().right);
		m_Button_F10->MoveTo(m_Button_F8->Frame().right,bottom);
		m_Button_F10->ResizeTo(buttonwidth,funcbtnsheight);
		m_Button_F10->Invalidate();
	}

	// Command line	
	if (!m_CommandLine->IsHidden())
	{
		const int cmdlineheight = 18;
		m_CommandLine->MoveTo(r.left+2,bottom-cmdlineheight-2);
		m_CommandLine->ResizeTo(r.right-4,cmdlineheight);
		m_CommandLine->Invalidate();
		bottom = bottom-cmdlineheight-2;
	}
	
	// Panels
	buttonwidth = (r.right-r.left-6)/2;
	
	m_LeftPanel->MoveTo(BPoint(r.left+2,r.top+22));
	m_LeftPanel->ResizeTo(buttonwidth-2,bottom-r.top-24);
	m_LeftPanel->FrameResized(0,0);

	m_RightPanel->MoveTo(BPoint(r.left+4+ (int)buttonwidth ,r.top+22));
	m_RightPanel->ResizeTo((r.right-r.left)-(int)buttonwidth-6,bottom-r.top-24);
	m_RightPanel->FrameResized(0,0);
	
	UpdateIfNeeded();
}

////////////////////////////////////////////////////////////////////////
PanelView *GenesisWindow::GetActivePanel(void)
////////////////////////////////////////////////////////////////////////
{
	if ((m_LeftPanel->m_LastSelectionTime) > (m_RightPanel->m_LastSelectionTime))
		return m_LeftPanel;
	else
		return m_RightPanel;
}

////////////////////////////////////////////////////////////////////////
PanelView *GenesisWindow::GetInactivePanel(void)
////////////////////////////////////////////////////////////////////////
{
	if ((m_LeftPanel->m_LastSelectionTime) > (m_RightPanel->m_LastSelectionTime))
		return m_RightPanel;
	else
		return m_LeftPanel;
}
////////////////////////////////////////////////////////////////////////
// Az aktiv panel-t aktivva rajzolja, a masikat (tobbit) inaktivva...
void GenesisWindow::UpdatePanels(void)
////////////////////////////////////////////////////////////////////////
{
	// Path 
	m_LeftPanel->m_PathStringView->SetHighColor(128,128,128);
	m_RightPanel->m_PathStringView->SetHighColor(128,128,128);

	PanelView *active = GetActivePanel();
	active->m_PathStringView->SetHighColor(0,0,0);

	m_LeftPanel->m_PathStringView->Invalidate();
	m_RightPanel->m_PathStringView->Invalidate();

	// CustomListView background color...
	m_LeftPanel->m_CustomListView->SetViewColor(216,216,216);
	m_RightPanel->m_CustomListView->SetViewColor(216,216,216);

	m_LeftPanel->m_CustomListView->SetSelectionColor(200,200,200);
	m_RightPanel->m_CustomListView->SetSelectionColor(200,200,200);
	
	active->m_CustomListView->SetSelectionColor(128,128,128);
	active->m_CustomListView->SetViewColor(180,190,200);	// 102, 152, 203

	m_LeftPanel->m_CustomListView->Invalidate();
	m_RightPanel->m_CustomListView->Invalidate();

	// reset shortcuts for [cd] menu items within active panel,
	// so by pressing the keyboard shortcut the directory will be changed always inside active panel
	BMenuItem *item;
	uint32 modifiers;
	char shortcut;

	for (int32 i = 0; i < active->m_PathMenu->CountItems(); i++)
	{
		item = active->m_PathMenu->ItemAt(i);
		if (item != NULL)
		{
			shortcut = item->Shortcut(&modifiers);
			if (shortcut != 0)
				item->SetShortcut(shortcut, modifiers);
		}
	}
	

	UpdateCommandLinePath();
}

////////////////////////////////////////////////////////////////////////
void GenesisWindow::SetMousePointer(MousePointer mp)
////////////////////////////////////////////////////////////////////////
{
	// Hourglass...
	unsigned char cursordata[] = {
			0x10,0x01,0x07,0x07,0x3F,0xF8,0x20,0x58,0x3F,0xF8,0x10,0x30,0x10,0x30,0x08,0x60,0x04,0xC0,0x03,0x80,0x04,0xC0,0x09,0x60,0x13,0xB0,0x17,0xF0,0x3F,0xF8,0x20,0x58,
			0x3F,0xF8,0x00,0x00,0x3F,0xF8,0x3F,0xF8,0x3F,0xF8,0x14,0x30,0x14,0x30,0x0A,0x60,0x04,0xC0,0x03,0x80,0x04,0xC0,0x09,0x60,0x13,0xB0,0x17,0xF0,0x3F,0xF8,0x3F,0xF8,
			0x3F,0xF8,0x00,0x00 };

	if (m_MousePointer!=mp)
	{
		m_MousePointerChanged = true;
		m_MousePointer = mp;
	
		switch (mp)
		{
			case CR_DEFAULT:
				((GenesisApp *)be_app)->SetCursor(B_CURSOR_SYSTEM_DEFAULT, true);
				break;
			case CR_HOURGLASS:
				BCursor cursor((void *)cursordata);
				((GenesisApp *)be_app)->SetCursor(&cursor);
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisWindow::SetMousePointerShape(int n)
////////////////////////////////////////////////////////////////////////
{
	// Hourglass...
	unsigned char cursordata[] = {
		0x10,0x01,0x07,0x07,0x3F,0xF8,0x20,0x58,0x3F,0xF8,0x10,0x30,0x10,0x30,0x08,0x60,0x04,0xC0,0x03,0x80,0x04,0xC0,0x09,0x60,0x13,0xB0,0x17,0xF0,0x3F,0xF8,0x20,0x58,
		0x3F,0xF8,0x00,0x00,0x3F,0xF8,0x3F,0xF8,0x3F,0xF8,0x14,0x30,0x14,0x30,0x0A,0x60,0x04,0xC0,0x03,0x80,0x04,0xC0,0x09,0x60,0x13,0xB0,0x17,0xF0,0x3F,0xF8,0x3F,0xF8,
		0x3F,0xF8,0x00,0x00 };
	
	switch (n)
	{
		case CR_DEFAULT:
			((GenesisApp *)be_app)->SetCursor(B_CURSOR_SYSTEM_DEFAULT);
			break;
		case CR_HOURGLASS:
			((GenesisApp *)be_app)->SetCursor((void *)cursordata);
			break;
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisWindow::UpdateCommandLinePath(void)
////////////////////////////////////////////////////////////////////////
{
	PanelView *active = GetActivePanel();
	m_CommandLine->SetPath(active->m_Path.String());
}




////////////////////////////////////////////////////////////////////////
EscapeFilter::EscapeFilter(BWindow *window, BMessage* msg):
	BMessageFilter(B_KEY_DOWN)
////////////////////////////////////////////////////////////////////////
{
	m_TargetWindow = window;
	m_MessageToSend = msg;
}

////////////////////////////////////////////////////////////////////////
EscapeFilter::~EscapeFilter()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
filter_result EscapeFilter::Filter(BMessage* msg, BHandler** target)
////////////////////////////////////////////////////////////////////////
{
    if (msg->what == B_KEY_DOWN)
    {
        char byte;
        if (msg->FindInt8("byte", (int8*)&byte) == B_OK)
        {
			if (m_TargetWindow)
			{
				switch(byte)
				{
					case B_ESCAPE:
						m_TargetWindow->Looper()->PostMessage(m_MessageToSend, NULL);
						return B_SKIP_MESSAGE;
					case B_FUNCTION_KEY:
					case B_INSERT:
						return B_SKIP_MESSAGE;
				}
			}
        }
    }
	return B_DISPATCH_MESSAGE;
}



////////////////////////////////////////////////////////////////////////
KeyboardFilter::KeyboardFilter(BWindow *window, BMessage* msg):
	BMessageFilter(B_KEY_DOWN)
////////////////////////////////////////////////////////////////////////
{
	m_TargetWindow = window;
	m_MessageToSend = msg;
}

////////////////////////////////////////////////////////////////////////
KeyboardFilter::~KeyboardFilter()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
filter_result KeyboardFilter::Filter(BMessage* msg, BHandler** target)
////////////////////////////////////////////////////////////////////////
{
    if (msg->what == B_KEY_DOWN)
    {
		uint32 modifiers;
		uint32 rawKeyChar = 0;
		uint8 byte = 0;
		int32 key = 0;
		
		GenesisWindow *window = (GenesisWindow *)m_TargetWindow;
		if (!window->m_CommandLine->IsHidden())
		{
			BTextView *cmdlinetextview = (BTextView *)window->m_CommandLine->TextView();

			if ( cmdlinetextview && cmdlinetextview->IsFocus() )
			{
				msg->FindInt32("modifiers", (int32 *)&modifiers);
				msg->FindInt32("raw_char", (int32 *)&rawKeyChar);
				msg->FindInt8("byte", (int8 *)&byte);
				msg->FindInt32("key", &key);

//				char buf[256];
//				sprintf(buf,"%d",key);
//				BAlert *myAlert = new BAlert("DebugInfo",buf,"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
//				myAlert->Go();										

				if (key == 1)		// ESC
					return B_SKIP_MESSAGE;	

				if (key>=2 && key<=13)	// Function keys
					return B_SKIP_MESSAGE;
					
				if (key == 15 || key == 31)	// scroll lock || insert
					return B_SKIP_MESSAGE;
					
				if (key == 71 || key == 91)
					m_TargetWindow->Looper()->PostMessage(m_MessageToSend, NULL);
			}
		}
    }
    
	return B_DISPATCH_MESSAGE;
}



////////////////////////////////////////////////////////////////////////
SeekFilter::SeekFilter(BWindow* window, PanelView *panel, BMessage* msg):
	BMessageFilter(B_KEY_DOWN)
////////////////////////////////////////////////////////////////////////
{
	m_TargetWindow = window;
	m_MessageToSend = msg;
	m_PV = panel;
}

////////////////////////////////////////////////////////////////////////
SeekFilter::~SeekFilter()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
filter_result SeekFilter::Filter(BMessage* msg, BHandler** target)
////////////////////////////////////////////////////////////////////////
{
    if (msg->what == B_KEY_DOWN)
    {
		uint32 modifiers;
		uint32 rawKeyChar = 0;
		uint8 byte = 0;
		int32 key = 0;
		
		if (m_PV->m_SeekTextControl==NULL) return B_DISPATCH_MESSAGE;
		
		BTextView *seektextview = m_PV->m_SeekTextControl->TextView();

		if ( seektextview && seektextview->IsFocus() )
		{
			msg->FindInt32("modifiers", (int32 *)&modifiers);
			msg->FindInt32("raw_char", (int32 *)&rawKeyChar);
			msg->FindInt8("byte", (int8 *)&byte);
			msg->FindInt32("key", &key);

			if (key == 1)		// ESC
			{
				m_PV->SeekModeOff();
				return B_SKIP_MESSAGE;
			}

//			char buf[256];
//			sprintf(buf,"hehe");
//			BAlert *myAlert = new BAlert("DebugInfo",buf,"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
//			myAlert->Go();
		}
    }
    
	return B_DISPATCH_MESSAGE;
}


