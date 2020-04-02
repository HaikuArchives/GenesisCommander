/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019-2020, Ondrej Čerman
 */

#include "GenesisPreferencesWindow.h"
#include "GenesisWindow.h"
#include "Settings.h"
#include "GenesisPanelView.h"
#include <View.h>
#include <Window.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Path.h>
#include <Roster.h>
#include <SeparatorView.h>

////////////////////////////////////////////////////////////////////////
GenesisPreferencesWindow::GenesisPreferencesWindow(BLooper* looper, BWindow *mainwindow) :
	BWindow(BRect(0,0,400,200), "Genesis Preferences", B_TITLED_WINDOW,
		B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
////////////////////////////////////////////////////////////////////////
{
	m_Looper = looper;
	AddToSubset(mainwindow);
	BView *settingsview = new BView("settingscontainer", B_WILL_DRAW);

	// Preferences
	m_ShowFunctionKeys = new BCheckBox("showfkeys", "Show function keys", new BMessage(PREFERENCES_CHANGED));
	m_ShowCommandLine = new BCheckBox("showfkeys", "Show command line", new BMessage(PREFERENCES_CHANGED));
	m_SymlinkedPaths = new BCheckBox("followsymlinks", "Keep symlinked directory paths when navigating", new BMessage(PREFERENCES_CHANGED));
	m_AskOnExit = new BCheckBox("askonexit", "Ask on exit", new BMessage(PREFERENCES_CHANGED));

	m_EditorsMenu = new BMenu("Select ...",B_ITEMS_IN_COLUMN);
	BMenuField *ChooseEditors = new BMenuField("",m_EditorsMenu);
	ChooseEditors->SetDivider(0);

	m_EditorApp = new BTextControl("editor", "Editor application (F4)", "", NULL, B_WILL_DRAW|B_NAVIGABLE);
	m_TerminalWindowTitle = new BTextControl("terminaltitle", "Terminal window title", "", NULL, B_WILL_DRAW|B_NAVIGABLE);
	m_LeftPanelPath = new BTextControl("leftpath", "Initial path of left panel", "", NULL, B_WILL_DRAW|B_NAVIGABLE);
	m_RightPanelPath = new BTextControl("rightpath", "Initial path of right panel", "", NULL, B_WILL_DRAW|B_NAVIGABLE);

	BButton *SetCurrLeftPathButton = new BButton("currleft", "Set to current", new BMessage(BUTTON_MSG_SET_CURR_PATH_L));
	SetCurrLeftPathButton->SetToolTip("Set initial path to current path of left panel");
	BButton *SetCurrRightPathButton = new BButton("currleft", "Set to current", new BMessage(BUTTON_MSG_SET_CURR_PATH_R));
	SetCurrRightPathButton->SetToolTip("Set initial path to current path of right panel");

	BGridLayout* settingsgrid = BGridLayoutBuilder(10, 10)
		.Add(m_TerminalWindowTitle->CreateLabelLayoutItem(), 0, 0)
		.Add(m_TerminalWindowTitle->CreateTextViewLayoutItem(), 1, 0)

		.Add(m_EditorApp->CreateLabelLayoutItem(), 0, 1)
		.Add(m_EditorApp->CreateTextViewLayoutItem(), 1, 1)
		.Add(ChooseEditors, 2, 1)

		.Add(m_LeftPanelPath->CreateLabelLayoutItem(), 0, 2)
		.Add(m_LeftPanelPath->CreateTextViewLayoutItem(), 1, 2)
		.Add(SetCurrLeftPathButton, 2, 2)

		.Add(m_RightPanelPath->CreateLabelLayoutItem(), 0, 3)
		.Add(m_RightPanelPath->CreateTextViewLayoutItem(), 1, 3)
		.Add(SetCurrRightPathButton, 2, 3);

	settingsgrid->SetMinColumnWidth(1, 200);

	BLayoutBuilder::Group<>(settingsview, B_VERTICAL, 0)
		.SetInsets(20)
		.Add(m_ShowFunctionKeys)
		.Add(m_ShowCommandLine)
		.Add(m_SymlinkedPaths)
		.Add(m_AskOnExit)
		.Add(BSpaceLayoutItem::CreateVerticalStrut(10))
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(BSpaceLayoutItem::CreateVerticalStrut(10))
		.Add(settingsgrid);

	// Bottom bar
	m_ApplyButton = new BButton("apply", "Apply", new BMessage(BUTTON_MSG_APPLY));
	m_ApplyButton->SetEnabled(false);

	BButton *CancelButton = new BButton("cancel", "Cancel", new BMessage(BUTTON_MSG_CANCEL));
	BView *bottomciew = new BView("infobottomview", B_WILL_DRAW);
	BLayoutBuilder::Group<>(bottomciew)
		.AddGroup(B_HORIZONTAL, 20)
			.SetInsets(20, 8, 20, 2)
			.AddGlue()
			.Add(CancelButton)
			.Add(m_ApplyButton)
			.End();

	bottomciew->SetViewColor(180, 190, 200, 0);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(settingsview)
		.Add(bottomciew);

	SetDefaultButton(m_ApplyButton);
	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCEL)));

	// If there is a given window, let's align our window to its center...
	if (mainwindow)
	{
		BRect myrect = Bounds();
		BRect rect = mainwindow->Frame();
		float w = rect.right - rect.left;
		float h = rect.bottom - rect.top;
		MoveTo(rect.left + w/2 - (myrect.right-myrect.left)/2, rect.top + h/2 - (myrect.bottom-myrect.top)/2);
	}

	ReloadSettings();

	m_TerminalWindowTitle->SetModificationMessage(new BMessage(PREFERENCES_CHANGED));
	m_LeftPanelPath->SetModificationMessage(new BMessage(PREFERENCES_CHANGED));
	m_RightPanelPath->SetModificationMessage(new BMessage(PREFERENCES_CHANGED));
	m_EditorApp->SetModificationMessage(new BMessage(PREFERENCES_CHANGED));

	FillEditors();
}

////////////////////////////////////////////////////////////////////////
GenesisPreferencesWindow::~GenesisPreferencesWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisPreferencesWindow::FillEditors()
////////////////////////////////////////////////////////////////////////
{
	BMenuItem *appitem;
	BMessage supportingApps;
	BMimeType mime("text/plain");
	if (mime.GetSupportingApps(&supportingApps) != B_OK)
		return;

	BMessage *msg;
	BRoster roster;
	BEntry appentry;
	BPath apppath;
	entry_ref appref;
	const char *appsig;

	for (int i = 0; supportingApps.FindString("applications", i, &appsig) == B_OK; i++)
	{
		if (roster.FindApp(appsig, &appref) == B_OK)
		{
			appentry.SetTo(&appref);
			appentry.GetPath(&apppath);

			msg = new BMessage(MENU_MSG_SET_EDITOR);
			msg->AddString("path", apppath.Path());

			appitem = new BMenuItem(apppath.Leaf(), msg, 0);
			m_EditorsMenu->AddItem(appitem);
		}
	}

}

////////////////////////////////////////////////////////////////////////
void GenesisPreferencesWindow::ReloadSettings()
////////////////////////////////////////////////////////////////////////
{
	m_ShowCommandLine->SetValue(SETTINGS->GetShowCommandLine() ? B_CONTROL_ON: B_CONTROL_OFF);
	m_ShowFunctionKeys->SetValue(SETTINGS->GetShowFunctionKeys() ? B_CONTROL_ON: B_CONTROL_OFF);
	m_SymlinkedPaths->SetValue(SETTINGS->GetSymlinkedPaths() ? B_CONTROL_ON: B_CONTROL_OFF);
	m_AskOnExit->SetValue(SETTINGS->GetAskOnExit() ? B_CONTROL_ON: B_CONTROL_OFF);
	m_TerminalWindowTitle->SetText(SETTINGS->GetTerminalWindow());
	m_EditorApp->SetText(SETTINGS->GetEditorApp());
	m_LeftPanelPath->SetText(SETTINGS->GetLeftPanelPath().String());
	m_RightPanelPath->SetText(SETTINGS->GetRightPanelPath().String());
	m_ApplyButton->SetEnabled(false);
}

////////////////////////////////////////////////////////////////////////
void GenesisPreferencesWindow::ApplySettings()
////////////////////////////////////////////////////////////////////////
{
	SETTINGS->SetShowCommandLine(m_ShowCommandLine->Value() == B_CONTROL_ON);
	SETTINGS->SetShowFunctionKeys(m_ShowFunctionKeys->Value() == B_CONTROL_ON);
	SETTINGS->SetSymlinkedPaths(m_SymlinkedPaths->Value() == B_CONTROL_ON);
	SETTINGS->SetAskOnExit(m_AskOnExit->Value() == B_CONTROL_ON);
	SETTINGS->SetTerminalWindow(m_TerminalWindowTitle->Text());
	SETTINGS->SetEditorApp(m_EditorApp->Text());
	SETTINGS->SetLeftPanelPath(m_LeftPanelPath->Text());
	SETTINGS->SetRightPanelPath(m_RightPanelPath->Text());
}

////////////////////////////////////////////////////////////////////////
void GenesisPreferencesWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case BUTTON_MSG_SET_CURR_PATH_L:
			m_LeftPanelPath->SetText(MAINWINDOW->m_LeftPanel->m_Path.String());
			m_LeftPanelPath->MakeFocus(true);
			break;
		case BUTTON_MSG_SET_CURR_PATH_R:
			m_RightPanelPath->SetText(MAINWINDOW->m_RightPanel->m_Path.String());
			m_RightPanelPath->MakeFocus(true);
			break;
		case PREFERENCES_CHANGED:
			m_ApplyButton->SetEnabled(true);
			break;
		case BUTTON_MSG_CANCEL:
			Close();
			break;
		case BUTTON_MSG_APPLY:
			ApplySettings();
			SETTINGS->SaveSettings();
			m_ApplyButton->SetEnabled(false);
			if (m_Looper != NULL)
				m_Looper->PostMessage(new BMessage(MSG_PREFERENCES_CHANGED), NULL);
			break;
		case MENU_MSG_SET_EDITOR:
			{
				BString app;
				if (message->FindString("path", &app) == B_OK)
					m_EditorApp->SetText(app);
				break;
			}
		default:
			BWindow::MessageReceived(message);
	}
}
