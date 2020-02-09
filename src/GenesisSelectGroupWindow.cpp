/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019-2020, Ondrej Čerman
 */

#include "GenesisPanelView.h"
#include "GenesisSelectGroupWindow.h"
#include "GenesisWindow.h"
#include <Alert.h>
#include <Beep.h>
#include <Button.h>
#include <Directory.h>
#include <fnmatch.h>
#include <Path.h>
#include <stdio.h>
#include <View.h>
#include <Window.h>

////////////////////////////////////////////////////////////////////////
GenesisSelectGroupWindow::GenesisSelectGroupWindow(CustomListView *list, BWindow *mainwindow, bool selectmode) :
	BWindow(BRect(0,0,320,100), "Select/deselect", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	m_CustomListView = list;
	m_Window = mainwindow;
	m_SelectMode = selectmode;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE);
	SetTitle(selectmode ? "Select group": "Deselect group");

	AddToSubset(mainwindow);

	m_View = new BView(Bounds(), "selectview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_View);

	// Bottom View
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "selectbottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);

	// Rename Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;
	m_ActionButton = new BButton(rect,"action",selectmode ? "Select": "Deselect",new BMessage(BUTTON_MSG_SELECT),0,B_WILL_DRAW);
	BottomView->AddChild(m_ActionButton);

	// Cancel Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;
	BButton *CancelButton = new BButton(rect,"cancel","Cancel",new BMessage(BUTTON_MSG_CANCELSELECT),0,B_WILL_DRAW);
	BottomView->AddChild(CancelButton);

	SetDefaultButton(m_ActionButton);

	// Edit field
	rect = BottomView->Bounds();
	rect.top = rect.top+20;
	rect.bottom = rect.top+32;
	rect.left += 20;
	rect.right -= 20;

	BString label = selectmode? "Select:": "Deselect:";
	BString pattern = (m_CustomListView->m_PV != NULL) ? ((PanelView*)m_CustomListView->m_PV)->m_LastFilePattern: BString("*.*");
	m_EntryPattern = new BTextControl(rect, "filepattern", label, pattern , NULL);
	m_EntryPattern->SetDivider(m_View->StringWidth(label)+4);
	m_EntryPattern->SetModificationMessage(new BMessage(ENTRYFILTER_CHANGED));
	m_View->AddChild(m_EntryPattern);

	m_EntryPattern->MakeFocus(true);

	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCELSELECT)));

	if (strlen(m_EntryPattern->Text())==0)
		m_ActionButton->SetEnabled(false);
	else
		m_ActionButton->SetEnabled(true);

	// If there is a given window, let's align our window to its center...
	if (mainwindow)
	{
		BRect myrect = Bounds();

		rect = mainwindow->Frame();
		float w = rect.right - rect.left;
		float h = rect.bottom - rect.top;
		MoveTo(rect.left + w/2 - (myrect.right-myrect.left)/2, rect.top + h/2 - (myrect.bottom-myrect.top)/2);
	}
}

////////////////////////////////////////////////////////////////////////
GenesisSelectGroupWindow::~GenesisSelectGroupWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisSelectGroupWindow::ChangeSelection(BString input)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	m_Window->Lock();

	int n = m_CustomListView->CountItems();
	for(int i=0; i<n; i++)
	{
		item = (CustomListItem *)m_CustomListView->ItemAt(i);
		if (item)
		{
			if (item->m_Type == FT_PARENT)
			{
				m_CustomListView->Deselect(i);
			}
			else{
				if (fnmatch(input, item->m_FileName, 0)==0)
				{
					if (m_SelectMode)
						m_CustomListView->Select(i,true);
					else
						m_CustomListView->Deselect(i);
				}
			}
		}
	}

	m_CustomListView->ScrollToSelection();
	m_Window->Unlock();
}

////////////////////////////////////////////////////////////////////////
void GenesisSelectGroupWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case ENTRYFILTER_CHANGED:
			if (strlen(m_EntryPattern->Text())>0)
				m_ActionButton->SetEnabled(true);
			else
				m_ActionButton->SetEnabled(false);
			break;
		case BUTTON_MSG_CANCELSELECT:
			Close();
			break;
		case BUTTON_MSG_SELECT:
			ChangeSelection(m_EntryPattern->Text());
			if (m_CustomListView->m_PV != NULL)
				((PanelView*)m_CustomListView->m_PV)->m_LastFilePattern = m_EntryPattern->Text();
			Close();
			break;
		default:
			BWindow::MessageReceived(message);
	}
}
