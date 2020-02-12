/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#include "GenesisMakeFileWindow.h"
#include "GenesisPanelView.h"
#include "GenesisWindow.h"
#include <stdio.h>
#include <View.h>
#include <Window.h>
#include <Alert.h>
#include <Beep.h>
#include <Button.h>
#include <Directory.h>

////////////////////////////////////////////////////////////////////////
GenesisMakeFileWindow::GenesisMakeFileWindow(const char* dirpath, BLooper* looper, BWindow *mainwindow, bool directory, bool edit) :
	BWindow(BRect(0,0,320,100), "Create new file", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	m_DirPath.SetTo(dirpath);
	m_Looper = looper;
	m_MkDirMode = directory;
	m_EditAfter = edit;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE);

	AddToSubset(mainwindow);

	if (m_MkDirMode)
		SetTitle("Create new folder");

	m_View = new BView(Bounds(), "makefileview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_View);

	// Bottom View
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);	//
	m_View->AddChild(BottomView);

	// OK Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;
	m_OkButton = new BButton(rect,"ok","Create",new BMessage(BUTTON_MSG_CREATE_FILE),0,B_WILL_DRAW);
	BottomView->AddChild(m_OkButton);

	//Cancel Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;
	BButton *CancelButton = new BButton(rect,"cancel","Cancel",new BMessage(BUTTON_MSG_CANCEL),0,B_WILL_DRAW);
	BottomView->AddChild(CancelButton);

	SetDefaultButton(m_OkButton);

	// Edit field
	rect = BottomView->Bounds();
	rect.top = rect.top+20;
	rect.bottom = rect.top+32;
	rect.left += 20;
	rect.right -= 20;
	m_FileName = new BTextControl( rect, "filename", "Name:", m_MkDirMode ? "New folder": "New file", NULL );
	m_FileName->SetDivider(m_View->StringWidth("Name:")+4);
	m_FileName->SetModificationMessage(new BMessage(FILENAME_CHANGED));
	m_View->AddChild(m_FileName);

	m_FileName->MakeFocus(true);

	// Ctrl + Q closes the window...
	AddShortcut('Q', 0, new BMessage(BUTTON_MSG_CANCEL));

	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCEL)));

	if (strlen(m_FileName->Text())==0)
		m_OkButton->SetEnabled(false);
	else
		m_OkButton->SetEnabled(true);

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
GenesisMakeFileWindow::~GenesisMakeFileWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisMakeFileWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case FILENAME_CHANGED:
			if (strlen(m_FileName->Text())>0)
				m_OkButton->SetEnabled(true);
			else
				m_OkButton->SetEnabled(false);
			break;
		case BUTTON_MSG_CANCEL:
			Close();
			break;
		case BUTTON_MSG_CREATE_FILE:
			if (strlen(m_FileName->Text())>0)
			{
				bool created;

				if (m_MkDirMode)
					created = CreateFolder(m_DirPath.String(), m_FileName->Text());
				else
					created = CreateFile(m_DirPath.String(), m_FileName->Text());

				if (created && m_Looper != NULL)
				{
					BMessage *msg = new BMessage(MSG_RELOAD);
					msg->AddString("ItemName",m_FileName->Text());
					m_Looper->PostMessage(msg, NULL);

					if (!m_MkDirMode && m_EditAfter)
					{
						msg = new BMessage(MSG_EDIT);
						msg->AddString("ItemName",m_FileName->Text());
						m_Looper->PostMessage(msg, NULL);
					}
					Close();
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
bool GenesisMakeFileWindow::CreateFolder(const char *dirpath, const char *dirname)
////////////////////////////////////////////////////////////////////////
{
	BString dir;
	dir.SetTo(dirpath);
	dir+="/";
	dir+=dirname;

	BString errormsg;
	BEntry *entry = new BEntry(dir, true);
	if (entry->Exists() && entry->IsDirectory())
	{
		errormsg.SetToFormat("The folder '%s' already exists.", dirname);
		BAlert *alert = new BAlert("Error creating folder",errormsg.String(),"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_INFO_ALERT);
		alert->Go();
		return 1;
	}

	status_t status = create_directory(dir.String(), 0777);
	if (status == B_OK)
		return 1;

	switch (status){
		case B_NOT_A_DIRECTORY:
			errormsg.SetToFormat("A file '%s' already exists.", dirname);
			break;
		default:
			errormsg.SetTo("Unknown error");
			break;
	}

	BAlert *alert = new BAlert("Error creating folder",errormsg.String(),"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT);
	alert->Go();

	return 0;
}

////////////////////////////////////////////////////////////////////////
bool GenesisMakeFileWindow::CreateFile(const char *path, const char *filename)
////////////////////////////////////////////////////////////////////////
{
	BDirectory *dir = new BDirectory(path);

	status_t status = dir->CreateFile(filename, NULL, true);
	if (status == B_OK)
		return 1;

	BString errormsg;
	switch (status){
		case B_FILE_EXISTS:
		{
			BString filepath;
			filepath.SetToFormat("%s/%s", path, filename);
			BEntry *entry = new BEntry(filepath, true);
			if (entry->IsDirectory())
				errormsg.SetToFormat("A folder '%s' already exists.", filename);
			else
				errormsg.SetToFormat("A file '%s' already exists.", filename);
			break;
		}
		default:
			errormsg.SetTo("Unknown error");
			break;
	}

	BAlert *alert = new BAlert("Error creating file",errormsg.String(),"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT);
	alert->Go();

	return 0;
}


////////////////////////////////////////////////////////////////////////
CustomTextControl::CustomTextControl(BRect rect, const char *name, const char *label, const char *text) :
	BTextControl(rect, name, label, text, NULL)
////////////////////////////////////////////////////////////////////////
{


}

////////////////////////////////////////////////////////////////////////
CustomTextControl::~CustomTextControl()
////////////////////////////////////////////////////////////////////////
{

}
/*
void CustomTextControl::KeyUp(const char *bytes, int32 numBytes)
{
	if (bytes[0]==B_ESCAPE)
	{
		beep();
	}
	else BTextControl::KeyDown(bytes, numBytes);
}
*/
////////////////////////////////////////////////////////////////////////
void CustomTextControl::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	beep();
	switch(message->what)
	{
		default:
			BTextControl::MessageReceived(message);
	}
}

/*
void CustomTextControl::SetESCMessage(BMessage *msg)
{
	escmsg = msg;
}
*/
