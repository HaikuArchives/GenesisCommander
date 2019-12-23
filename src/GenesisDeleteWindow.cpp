/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisDeleteWindow.h"
#include "GenesisPanelView.h"
#include "GenesisWindow.h"
#include <stdio.h>
#include <Window.h>
#include <Button.h>
#include <Beep.h>
#include <Directory.h>
#include <Path.h>
#include <Entry.h>
#include <Autolock.h>
#include <Alert.h>

const int kSemTimeOut= 50000;

////////////////////////////////////////////////////////////////////////
GenesisDeleteWindow::GenesisDeleteWindow(CustomListView *list, BLooper* looper, BWindow *mainwindow) :
	BWindow(BRect(0,0,320,140), "Delete in progress...", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	rgb_color BarColor;
	BarColor.red = 180;
	BarColor.green = 190;
	BarColor.blue = 200;

	m_CustomListView = list;
	m_Looper = looper;
	m_Window = mainwindow;

	m_Paused = false;
	m_SkipAllError = false;
	m_DeleteAllNotEmpty = false;

	((PanelView *)m_CustomListView->m_PV)->DisableMonitoring();

	// After the delete process we have to select an item if no item selected...
	m_Selection = GetFirstSelection();

	// First we have to remove the parent selection if selected...
	RemoveParentSelection();

	m_FileCount = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);			

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE);

	AddToSubset(mainwindow);

	m_View = new BView(Bounds(), "makedirview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_View);

	// Bottom View	
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);	
	
	// Abort Button	
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;	
	m_AbortButton = new BButton(rect,"abort","Abort",new BMessage(BUTTON_MSG_ABORT),0,B_WILL_DRAW);
	BottomView->AddChild(m_AbortButton);

	//Pause Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;	
	m_PauseButton = new BButton(rect,"pause","Pause",new BMessage(BUTTON_MSG_PAUSE),0,B_WILL_DRAW);
	BottomView->AddChild(m_PauseButton);

	SetDefaultButton(m_AbortButton);

	// ProgressBar
	rect = Bounds();
	rect.left += 24;
	rect.right -= 24;
	rect.top += 32;
	rect.bottom = rect.top+40;
	m_ProgressBar = new BStatusBar(rect,"progressbar","","");
	m_ProgressBar->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	m_ProgressBar->SetBarColor(BarColor);
	m_ProgressBar->SetMaxValue(m_FileCount);
	AddChild(m_ProgressBar);
/*
	rect = Bounds();
	rect.left += 24;
	rect.right -= 24;
	rect.top += 60;
	rect.bottom = rect.top+20;
	m_Label = new BStringView(rect,"filename","Deleting...");
	m_Label->SetAlignment(B_ALIGN_CENTER);
	m_Label->SetViewColor(216,216,216);
	m_Label->SetLowColor(216,216,216);
	m_Label->SetHighColor(0,0,0);
	AddChild(m_Label);
*/
	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_ABORT)));

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
GenesisDeleteWindow::~GenesisDeleteWindow()
////////////////////////////////////////////////////////////////////////
{
	if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==0)
	{
		if (m_Selection > (m_CustomListView->IndexOf(m_CustomListView->LastItem())))
			m_Selection = m_CustomListView->IndexOf(m_CustomListView->LastItem());
			
		m_CustomListView->Select(m_Selection, false);	// false -> remove previously selected item(s)...
	}
	
	((PanelView *)m_CustomListView->m_PV)->EnableMonitoring();
}

////////////////////////////////////////////////////////////////////////
void GenesisDeleteWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case BUTTON_MSG_ABORT:
			{
				BAlert *myAlert = new BAlert("Delete","Do you really want to abort?","No","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
				myAlert->SetShortcut(0, B_ESCAPE);
				if (myAlert->Go()==1)
				{
					kill_thread(m_DeleteThread);
					Close();
				}
			}
			break;
		case BUTTON_MSG_PAUSE:
			if (m_Paused)
			{
				if (resume_thread(m_DeleteThread)==B_OK)
				{
					m_PauseButton->SetLabel("Pause");
					m_Paused = false;
				}
			}
			else
			{
				if (suspend_thread(m_DeleteThread)==B_OK)
				{
					m_PauseButton->SetLabel("Resume");
					m_Paused = true;
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisDeleteWindow::RemoveParentSelection()
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	item = m_CustomListView->GetSelectedEntry(0);
	if (item && item->m_Type==FT_PARENT)
	{
		m_CustomListView->Deselect(0);
		m_CustomListView->InvalidateItem(0);
	}
}

////////////////////////////////////////////////////////////////////////
int32 DeleteThreadFunc(void *data)
////////////////////////////////////////////////////////////////////////
{
	GenesisDeleteWindow *d;
	BString filename;
	BString labeltext;
	CustomListItem *item;

	d = (GenesisDeleteWindow *)data;

	for(int i=0;i<(d->m_FileCount);i++)
	{
		item = (CustomListItem *)d->m_CustomListView->GetSelectedEntry(0);
		if (item)
		{
			filename.SetTo(item->m_FileName); // = (BString *)d->m_FileList->ItemAt(i);
/*
			d->Lock();
			labeltext.SetTo("Deleting: ");
			labeltext << filename.String();
			d->m_ProgressBar->SetText(labeltext.String());
//			d->m_Label->SetText(labeltext.String());
			d->Unlock();
			
			d->UpdateIfNeeded();			// Delete window
*/
			filename.SetTo(item->m_FilePath.String());
			filename+="/";
			filename+=item->m_FileName;			

			if (d->UniversalDelete(filename.String())==B_OK)
			{
				d->m_Window->Lock();
				d->m_CustomListView->RemoveItem(item);
				((PanelView *)d->m_CustomListView->m_PV)->m_CurrentTotalSize = d->m_CustomListView->GetCurrentTotalSize();
				d->m_Window->Unlock();
			}

			d->Lock();
			d->m_ProgressBar->Update(1);
			d->Unlock();
		}
	}
	
	d->Lock();
	d->m_ProgressBar->SetText("Done");
	d->m_AbortButton->SetEnabled(false);
	d->m_PauseButton->SetEnabled(false);
	d->Unlock();
	d->UpdateIfNeeded();			// Delete window
	snooze(600000);

	d->Close();

	return B_OK;
}

////////////////////////////////////////////////////////////////////////
void GenesisDeleteWindow::Go(void)
////////////////////////////////////////////////////////////////////////
{
	Show();

	BAutolock autolocker(this);

	m_DeleteThread = spawn_thread(DeleteThreadFunc, "DeleteThread", B_NORMAL_PRIORITY, (void *)this);
	resume_thread(m_DeleteThread);
}

////////////////////////////////////////////////////////////////////////
void GenesisDeleteWindow::DeleteDirectory(const char *dirname)
////////////////////////////////////////////////////////////////////////
{
	BDirectory *dir;
	
	// Don't delete the parent directory!!!!!!
	if (strlen(dirname)>=3)
	{
		int len = strlen(dirname);
		if (dirname[len-1]=='.' && dirname[len-2]=='.' && dirname[len-3]=='/') return;
	}
		
	dir = new BDirectory(dirname);
	if (dir)
	{
		BEntry entry;

		if (dir->CountEntries()>0)
		{
			if (!m_DeleteAllNotEmpty) FolderNotEmpty(dirname);
		}
		
		if (dir->GetEntry(&entry)==B_OK)
		{	
			while (dir->GetNextEntry(&entry)==B_OK)			
			{
				BPath path;
				entry.GetPath(&path);
				
				if (entry.IsDirectory())
					DeleteDirectory(path.Path());

				BString labeltext;
				char name[B_FILE_NAME_LENGTH];
				entry.GetName(name);
				
				Lock();
				labeltext.SetTo(name);
				m_ProgressBar->SetTrailingText(labeltext.String());
				Unlock();

				if (entry.Remove()!=B_OK)
				{
					if (!m_SkipAllError) DeleteError(dirname);
				}
			}
		}
	
		delete dir;
	}

	Lock();
	m_ProgressBar->SetTrailingText("");
	Unlock();
}

////////////////////////////////////////////////////////////////////////
bool GenesisDeleteWindow::UniversalDelete(const char *filename)
////////////////////////////////////////////////////////////////////////
{
	BEntry entry(filename);

	// Don't delete the parent directory!!!!!!
	if (strlen(filename)>=3)
	{
		int len = strlen(filename);
		if (filename[len-1]=='.' && filename[len-2]=='.' && filename[len-3]=='/') return B_ERROR;
	}

	if (entry.InitCheck()==B_OK)
	{
		if (entry.Exists())
		{
			BString labeltext;
			char name[B_FILE_NAME_LENGTH];
			entry.GetName(name);
			
			Lock();
			labeltext.SetTo("Deleting: ");
			labeltext << name;
			m_ProgressBar->SetText(labeltext.String());
			Unlock();

			if (entry.IsDirectory())
				DeleteDirectory(filename);
		
			if (entry.Remove()!=B_OK)
			{
				if (!m_SkipAllError) DeleteError(filename);
				return B_ERROR;
			}
		}
	}

	return B_OK;
}

////////////////////////////////////////////////////////////////////////
void GenesisDeleteWindow::DeleteError(const char *filename)
////////////////////////////////////////////////////////////////////////
{
	BString text;
	BEntry file(filename);
	char name[B_FILE_NAME_LENGTH];
	
	file.GetName(name);
	
	text << "Cannot delete the following entry:\n\n" << name;
	
	BAlert *myAlert = new BAlert("Delete",text.String(),"Abort","Skip all","Skip",B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
	myAlert->SetShortcut(0, B_ESCAPE);
	switch (myAlert->Go())
	{
		case 0:
			kill_thread(m_DeleteThread);
			Close();
			break;
		case 1:
			m_SkipAllError = true;
			break;
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisDeleteWindow::FolderNotEmpty(const char *dirname)
////////////////////////////////////////////////////////////////////////
{
	BString text;
	
	text << "The following directory is not empty:\n\n" << dirname;
	
	BAlert *myAlert = new BAlert("Delete",text.String(),"Abort","Delete all","Delete",B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
	myAlert->SetShortcut(0, B_ESCAPE);
	switch (myAlert->Go())
	{
		case 0:
			kill_thread(m_DeleteThread);
			Close();
			break;
		case 1:
			m_DeleteAllNotEmpty = true;
			break;
	}
}

////////////////////////////////////////////////////////////////////////
int32 GenesisDeleteWindow::GetFirstSelection(void)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	
	item = m_CustomListView->GetSelectedEntry(0);
	if (item)
		return m_CustomListView->IndexOf(item);
	else
		return 0;	
}
