/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisViewWindow.h"
#include <stdio.h>
#include <View.h>
#include <Window.h>
#include <File.h>
#include <Beep.h>
#include <Alert.h>
#include <Font.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Clipboard.h>

////////////////////////////////////////////////////////////////////////
GenesisViewWindow::GenesisViewWindow(const char* filename, BWindow *mainwindow) :
	BWindow(BRect(100,100,100+80*8,100+25*16), "View", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;
	char buf[B_FILE_NAME_LENGTH];
	BEntry entry;
	BString title;
	BFont font;
	BMenu *menu;
	
	m_FileName.SetTo(filename);

	entry.SetTo(filename);
	entry.GetName(buf);
	title.SetTo("View: ");
	title << buf;
	SetTitle(title.String());

	m_View = new BView(Bounds(), "view", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewColor(216, 216, 216, 0);
	AddChild(m_View);

	// Menu
	m_MenuBar = new BMenuBar(Bounds(),"mainmenu");

	// File menu
	menu = new BMenu("File");
	menu->AddItem(new BMenuItem("Save as..." , new BMessage(VIEWMENU_FILE_SAVEAS), 0));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Close" , new BMessage(VIEWMENU_FILE_CLOSE), 'Q'));
	m_MenuBar->AddItem(menu);

	menu = new BMenu("Edit");
	menu->AddItem(new BMenuItem("Copy" , new BMessage(VIEWMENU_FILE_COPY), 'C'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Select All" , new BMessage(VIEWMENU_FILE_SELECTALL), 'A'));
	m_MenuBar->AddItem(menu);
/*
	menu = new BMenu("Format");
	m_MI_WordWrap = new BMenuItem("Wrap text" , new BMessage(VIEWMENU_FILE_WORDWRAP), 0);
	menu->AddItem(m_MI_WordWrap);
	m_MenuBar->AddItem(menu);
*/
	m_View->AddChild(m_MenuBar);	

	// TextView
	rect = Bounds();
	rect.top+=m_MenuBar->Bounds().bottom+1;
	rect.right-=14;
	rect.bottom-=20;

	m_TextView = new CustomTextView(rect, "textview");
	m_TextView->SetViewColor(255,255,255);
	m_TextView->MakeEditable(false);
	m_TextView->SetStylable(true);

//	m_TextView->SetWordWrap(false);	// TODO: Set default value here from some settings...

	font = be_fixed_font;
	font.SetSize(12.0);
	m_TextView->SetFontAndColor(&font);

	BFile *file = new BFile(m_FileName.String(), B_READ_ONLY);
	if (file)
	{
		off_t size;
		file->GetSize(&size);

		if (size>262144) size = 262144;

		m_TextView->SetText(file,0,size);
		delete file;
	}	

	m_ScrollView = new BScrollView("scrollview", m_TextView, B_FOLLOW_ALL, B_WILL_DRAW, false, true);
	m_View->AddChild(m_ScrollView);

	// If there is a given window, let's align our window to its center...
	if (mainwindow)
	{
		BRect myrect = Bounds();
		
		rect = mainwindow->Frame();
		float w = rect.right - rect.left;
		float h = rect.bottom - rect.top;
		MoveTo(rect.left + w/2 - (myrect.right-myrect.left)/2, rect.top + h/2 - (myrect.bottom-myrect.top)/2);
	}	
	
	m_TextView->MakeFocus(true);
}

////////////////////////////////////////////////////////////////////////
GenesisViewWindow::~GenesisViewWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisViewWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case VIEWMENU_FILE_CLOSE:
			Close();
			break;
		case VIEWMENU_FILE_COPY:
			m_TextView->Copy(be_clipboard);
			break;
		case VIEWMENU_FILE_SELECTALL:
			m_TextView->SelectAll();
			break;
		case VIEWMENU_FILE_WORDWRAP:
			if (m_MI_WordWrap)
			{
				if (m_MI_WordWrap->IsMarked())
				{
					m_MI_WordWrap->SetMarked(false);
					m_TextView->SetWordWrap(false);
				}
				else
				{
					m_MI_WordWrap->SetMarked(true);
					m_TextView->SetWordWrap(true);
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}	
}
