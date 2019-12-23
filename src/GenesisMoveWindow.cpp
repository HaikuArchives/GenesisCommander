/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisMoveWindow.h"
#include "GenesisPanelView.h"
#include "GenesisWindow.h"
#include <stdio.h>
#include <fs_attr.h>
#include <View.h>
#include <Window.h>
#include <Beep.h>
#include <Button.h>
#include <Directory.h>
#include <List.h>
#include <File.h>
#include <Path.h>
#include <SymLink.h>
#include <String.h>
#include <Alert.h>
#include <Autolock.h>
#include <Node.h>
#include <NodeInfo.h>

////////////////////////////////////////////////////////////////////////
GenesisMoveWindow::GenesisMoveWindow(CustomListView *list, PanelView *destpanel, const char *destination, BLooper* looper, BWindow *mainwindow) :
	BWindow(BRect(0,0,320,140), "Move...", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	m_CustomListView = list;
	m_DestPanel = destpanel;
	m_DestPath.SetTo(destination);
	m_Looper = looper;
	m_Window = mainwindow;

	m_SingleMove = false;
	m_Paused = false;
	m_SkipAllMoveError = false;
	m_OverwriteAll = false;
	m_SkipSymLinkCreationError = false;
	m_SkipAllMissing = false;

	// After the delete process we have to select an item if no item selected...
	m_Selection = GetFirstSelection();

	// First we have to remove the parent selection if selected...
//	RemoveParentSelection();

	m_FileCount = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);			

	if (m_FileCount == 1)
		m_SingleMove = true;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE);

	AddToSubset(mainwindow);

	m_View = new BView(Bounds(), "moveview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_View);

	// Bottom View	
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);	
	
	// Move Button	
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;	
	m_MoveButton = new BButton(rect,"move","Move",new BMessage(BUTTON_MSG_MOVE),0,B_WILL_DRAW);
//	m_CopyButton->SetEnabled(false);
	BottomView->AddChild(m_MoveButton);

	//Cancel Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;	
	m_CancelButton = new BButton(rect,"cancel","Cancel",new BMessage(BUTTON_MSG_CANCELMOVE),0,B_WILL_DRAW);
	BottomView->AddChild(m_CancelButton);

	SetDefaultButton(m_MoveButton);

	// Info string
	rect = Bounds();
	rect.left += 24;
	rect.right -= 24;
	if (m_SingleMove)
		rect.top += 8;
	else
		rect.top += 16;
	rect.bottom = rect.top+20;
	m_Label = new BStringView(rect,"filename","Move");
	m_Label->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_Label);

	// Edit field
	rect = BottomView->Bounds();
	if (m_SingleMove)
		rect.top = rect.top+36;
	else
		rect.top = rect.top+56;
	rect.bottom = rect.top+32;
	rect.left += 20;
	rect.right -= 20;
	m_DirName = new BTextControl( rect, "destname", "to", "", NULL );
	m_DirName->SetDivider(m_View->StringWidth("to")+4);
	m_DirName->SetModificationMessage(new BMessage(MOVENAME_CHANGED));
	m_DirName->SetText(m_DestPath.String());
	m_View->AddChild(m_DirName);

	// "as" field
	if (m_SingleMove)
	{
		rect = BottomView->Bounds();
		rect.top = rect.top+64;
		rect.bottom = rect.top+32;
		rect.left += 20;
		rect.right -= 20;

		m_FileAsName = new BTextControl( rect, "destfilename", "as", "", NULL );
		m_FileAsName->SetDivider(m_View->StringWidth("to")+4);
		m_FileAsName->SetModificationMessage(new BMessage(MOVENAME_CHANGED));

		if (((CustomListItem *)m_CustomListView->GetSelectedEntry(0))->m_Type == FT_PARENT)
			m_FileAsName->SetText( ((CustomListItem *)m_CustomListView->GetSelectedEntry(1))->m_FileName.String() );
		else
			m_FileAsName->SetText( ((CustomListItem *)m_CustomListView->GetSelectedEntry(0))->m_FileName.String() );
		
		m_View->AddChild(m_FileAsName);
	}

	m_DirName->MakeFocus(true);	
	
	// Ctrl + Q closes the window...
	AddShortcut('Q', 0, new BMessage(BUTTON_MSG_CANCELMOVE));

	// Set move label...
	if (m_SingleMove)
	{
		BString text;
		text.SetTo("Move '");
		if (((CustomListItem *)m_CustomListView->GetSelectedEntry(0))->m_Type == FT_PARENT)
			text << ((CustomListItem *)m_CustomListView->GetSelectedEntry(1))->m_FileName;
		else
			text << ((CustomListItem *)m_CustomListView->GetSelectedEntry(0))->m_FileName;
		text << "'";
	
		m_Label->SetText(text.String());
	}
	else
	{
		BString text;
		text.SetTo("Move ");
		text << m_FileCount << " files";
		
		m_Label->SetText(text.String());
	}
	
	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCELMOVE)));
	
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
GenesisMoveWindow::~GenesisMoveWindow()
////////////////////////////////////////////////////////////////////////
{
	if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==0)
	{
		if (m_Selection > (m_CustomListView->IndexOf(m_CustomListView->LastItem())))
			m_Selection = m_CustomListView->IndexOf(m_CustomListView->LastItem());
			
		m_CustomListView->Select(m_Selection, false);	// false -> remove previously selected item(s)...
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisMoveWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case MOVENAME_CHANGED:

			if (m_SingleMove)	// Csak egy file van es akkor mind a ket mezot figyelni kell...
			{
				if (strlen(m_DirName->Text())>0 && strlen(m_FileAsName->Text())>0)
					m_MoveButton->SetEnabled(true);
				else
					m_MoveButton->SetEnabled(false);
			}
			else
			{
				if (strlen(m_DirName->Text())>0)
					m_MoveButton->SetEnabled(true);
				else
					m_MoveButton->SetEnabled(false);
			}
			break;
		case BUTTON_MSG_CANCELMOVE:
			if (find_thread("MoveThread")!=B_NAME_NOT_FOUND)	// Ha mar megy a atmozgatas...
			{
				BAlert *myAlert = new BAlert("Move","Do you really want to abort?","No","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
				myAlert->SetShortcut(0, B_ESCAPE);
				if (myAlert->Go()==1)
				{
					kill_thread(m_MoveThread);
					Close();
				}
			}
			else
				Close();
			break;
		case BUTTON_MSG_MOVE:
			PrepareMove();
			break;
		case BUTTON_MSG_PAUSEMOVE:
			if (m_Paused)
			{
				if (resume_thread(m_MoveThread)==B_OK)
				{
					m_PauseButton->SetLabel("Pause");
					m_Paused = false;
				}
			}
			else
			{
				if (suspend_thread(m_MoveThread)==B_OK)
				{
					m_PauseButton->SetLabel("Resume");
					m_Paused = true;
				}
			}
			break;
		case BUTTON_MSG_ABORTMOVE:
			{
				BAlert *myAlert = new BAlert("Copy","Do you really want to abort?","No","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
				myAlert->SetShortcut(0, B_ESCAPE);
				if (myAlert->Go()==1)
				{
					kill_thread(m_MoveThread);
					Close();
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisMoveWindow::RemoveParentSelection()
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	item = m_CustomListView->GetSelectedEntry(0);
	if (item && item->m_Type==FT_PARENT)
	{
		m_Window->Lock();
		m_CustomListView->Deselect(0);
		m_CustomListView->InvalidateItem(0);
		m_Window->Unlock();
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisMoveWindow::Go(void)
////////////////////////////////////////////////////////////////////////
{
	Show();
}

////////////////////////////////////////////////////////////////////////
int32 SingleMoveThreadFunc(void *data)
////////////////////////////////////////////////////////////////////////
{
	GenesisMoveWindow *d;
	BString filename;
	BString text;
	CustomListItem *item;

	d = (GenesisMoveWindow *)data;

	item = (CustomListItem *)d->m_CustomListView->GetSelectedEntry(0);
	if (item)
	{
		filename.SetTo(item->m_FileName); // = (BString *)d->m_FileList->ItemAt(i);
		text.SetTo("");
		text << "Moving '" << filename.String() << "' as '" << d->m_DestFileName.String() << "'";
		d->Lock();
		d->m_FileBar->SetText(text.String());
		d->Unlock();
		
		filename.SetTo(item->m_FilePath);
		filename += '/';
		filename += item->m_FileName;
		d->Move(filename.String(), d->m_DestPath.String(), d->m_DestFileName.String());

//		d->Lock();
//		d->m_ProgressBar->Update(1);
//		d->Unlock();

		d->m_Window->Lock();
		d->m_CustomListView->Deselect(d->m_CustomListView->IndexOf(item));
		d->m_CustomListView->InvalidateItem(d->m_CustomListView->IndexOf(item));
		d->m_Window->Unlock();
	}

	d->Lock();
	d->m_FileBar->SetText("Done");
	d->m_FileBar->SetTrailingText("");
	d->m_AbortButton->SetEnabled(false);
	d->m_PauseButton->SetEnabled(false);
	d->Unlock();
	d->UpdateIfNeeded();			// Delete window
	snooze(600000);

	d->Close();

	return B_OK;
}

////////////////////////////////////////////////////////////////////////
int32 MultiMoveThreadFunc(void *data)
////////////////////////////////////////////////////////////////////////
{
	GenesisMoveWindow *d;
	BString filename;
	BString text;
	CustomListItem *item;

	d = (GenesisMoveWindow *)data;

	for(int i=0;i<(d->m_FileCount);i++)
	{
		item = (CustomListItem *)d->m_CustomListView->GetSelectedEntry(0);
		if (item)
		{
			filename.SetTo(item->m_FileName); // = (BString *)d->m_FileList->ItemAt(i);
			text.SetTo("");
			text << "Moving '" << filename.String() << "'";
			d->Lock();
			d->m_FileBar->SetText(text.String());
			d->Unlock();
			
			filename.SetTo(item->m_FilePath);
			filename += '/';
			filename += item->m_FileName;
			if (d->Move(filename.String(), d->m_DestPath.String()))
			{	
				// successful move... le lehet szedni a listarol!
				d->m_Window->Lock();
				d->m_CustomListView->RemoveItem(item);	
				d->m_Window->Unlock();		
			}
			else
			{
				d->m_Window->Lock();
				d->m_CustomListView->Deselect(d->m_CustomListView->IndexOf(item));
				d->m_CustomListView->InvalidateItem(d->m_CustomListView->IndexOf(item));
				d->m_Window->Unlock();
			}
			
			d->Lock();
			d->m_ProgressBar->Update(1);
			d->Unlock();
		}
	}
	
	d->Lock();
	d->m_FileBar->SetText("Done");
	d->m_FileBar->SetTrailingText("");
	d->m_AbortButton->SetEnabled(false);
	d->m_PauseButton->SetEnabled(false);
	d->Unlock();
	d->UpdateIfNeeded();			// Delete window
	snooze(600000);

	d->Close();

	return B_OK;
}

////////////////////////////////////////////////////////////////////////
void GenesisMoveWindow::PrepareMove(void)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;
	
	rgb_color BarColor;
	BarColor.red = 180;
	BarColor.green = 190;
	BarColor.blue = 200;	

	m_DestPath.SetTo(m_DirName->Text());
	
	if (m_SingleMove)
		m_DestFileName.SetTo(m_FileAsName->Text());

	if (IsDirReadOnly(m_DestPath.String()))
	{
		BAlert *myAlert = new BAlert("Move", "Cannot move to a write protected volume.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);
		myAlert->Go();
		return;
	}

	BEntry dest(m_DestPath.String());
	
	if (dest.InitCheck()!=B_OK)
	{
		BAlert *myAlert = new BAlert("Move","Cannot initialize destination entry.","OK", NULL, NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->Go();
		return;
	}
	
	if (!dest.Exists())
	{
		BAlert *myAlert = new BAlert("Move","Destination path does not exist.","OK", NULL, NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->Go();
		return;
	}
	
	if (!dest.IsDirectory())
	{
		BAlert *myAlert = new BAlert("Move","Destination path is not a folder.","OK", NULL, NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->Go();
		return;
	}

	BAutolock autolocker(this);
	
	m_AbortButton = m_MoveButton;
	m_AbortButton->SetMessage(new BMessage(BUTTON_MSG_ABORTMOVE));
	m_PauseButton = m_CancelButton;
	m_PauseButton->SetMessage(new BMessage(BUTTON_MSG_PAUSEMOVE));
	
	m_DirName->RemoveSelf();
	m_Label->RemoveSelf();				//	m_Label->MoveBy(0,40);
	
	if (m_SingleMove)
		m_FileAsName->RemoveSelf();
	
	SetTitle("Move in progress...");
	
	// Add ProgressBar
	if (!m_SingleMove)
	{
		rect = Bounds();
		rect.left += 24;
		rect.right -= 24;
		rect.top += 8;
		rect.bottom = rect.top+40;
		m_ProgressBar = new BStatusBar(rect,"progressbar","0%","100%");
		m_ProgressBar->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
		m_ProgressBar->SetBarColor(BarColor);
		m_ProgressBar->SetMaxValue(m_FileCount);
		m_View->AddChild(m_ProgressBar);
	}
	
	// Add File Bar
	rect = Bounds();
	rect.left += 24;
	rect.right -= 24;
	if (m_SingleMove)
		rect.top += 32;
	else
		rect.top += 48;
	rect.bottom = rect.top+40;
	m_FileBar = new BStatusBar(rect,"filebar","","");
	m_FileBar->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	m_FileBar->SetBarColor(BarColor);
	m_View->AddChild(m_FileBar);

	m_AbortButton->SetLabel("Abort");
	m_PauseButton->SetLabel("Pause");

	RemoveParentSelection();

	if (m_SingleMove)
		m_MoveThread = spawn_thread(SingleMoveThreadFunc, "MoveThread", B_NORMAL_PRIORITY, (void *)this);
	else
		m_MoveThread = spawn_thread(MultiMoveThreadFunc, "MoveThread", B_NORMAL_PRIORITY, (void *)this);
	resume_thread(m_MoveThread);
}

////////////////////////////////////////////////////////////////////////
bool GenesisMoveWindow::Move(const char *filename, const char *destination, const char *destfilename)
////////////////////////////////////////////////////////////////////////
{
	bool result = false;	// Set true when move has been executed correctly...
	
	BEntry sourcefile(filename);
	BEntry dstfile;
	BDirectory destdir;
	BString text;
	BString destfullname;
	char name[B_FILE_NAME_LENGTH];
	bool overwrite = false;

	sourcefile.GetName(name);

	destfullname.SetTo(destination);
	destfullname += "/";
	if (destfilename)
		destfullname += destfilename;
	else
		destfullname += name;

	Lock();
	m_FileBar->Update(-m_FileBar->CurrentValue());	// Reset to 0.0
	m_FileBar->SetMaxValue(1);
	m_FileBar->SetTrailingText(name);
	Unlock();

	if (sourcefile.InitCheck()==B_OK)
	{
		if (!sourcefile.Exists() && m_SkipAllMissing==false)
		{
			BString text;
			text << "Source file does not exist.\n";
		
			BAlert *myAlert = new BAlert("Move",text.String(),"Abort", "Skip all", "Skip", B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
			switch (myAlert->Go())
			{
				case 0:
					Close();
					kill_thread(m_MoveThread);
					break;
				case 1:
					m_SkipAllMissing = true;
					break;
				case 2:
					return result;
			}	
		}

		if (sourcefile.IsDirectory())
		{
			if (IsRecursiveMove(filename, destination))
			{
				BString text;
				text << "Recursive move not allowed.\nPlease check the destination folder.";
			
				BAlert *myAlert = new BAlert("Move",text.String(),"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
				myAlert->Go();
				Close();
				kill_thread(m_MoveThread);
			}
		}

		if (m_OverwriteAll)
		{
			overwrite = true;
		}
		else
		{
			dstfile.SetTo(destfullname.String());
			if (dstfile.InitCheck()==B_OK && dstfile.Exists())
			{
				BString text;
				dstfile.GetName(name);
			
				text << "File '" << name << "' already exists. Do you want to overwrite it?";

				BAlert *myAlert = new BAlert("Move",text.String(),"Abort","Overwrite all","Overwrite",B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
				myAlert->SetShortcut(0, B_ESCAPE);
				switch (myAlert->Go())
				{
					case 0:
						Close();
						kill_thread(m_MoveThread);
						break;
					case 1:
						m_OverwriteAll = true;
						overwrite = true;
						break;
					case 2:
						overwrite = true;
						break;
				}			
			}
		}
					
		destdir.SetTo(destination);
		if (sourcefile.MoveTo(&destdir, destfilename, overwrite)==B_OK)		// Move!!!
			result = true;
		else if (!m_SkipAllMoveError)
		{
			text << "Cannot move '" << name << "' to \n\n" << destfullname << "\n\n";
			text << "Notice that move function works only when the source and destination folder are on the same volume.\n";
			
			BAlert *myAlert = new BAlert("Move",text.String(),"Abort","Skip all","Skip",B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
			myAlert->SetShortcut(0, B_ESCAPE);
			switch (myAlert->Go())
			{
				case 0:
					Close();
					kill_thread(m_MoveThread);
					break;
				case 1:
					m_SkipAllMoveError = true;
					break;
			}		
		}
		
		Lock();
		m_FileBar->Update(1);
		Unlock();
	}
	else if (!m_SkipAllMoveError)
	{
		text << "Error while initializing file:\n\n" << filename;
		
		BAlert *myAlert = new BAlert("Move",text.String(),"Abort","Skip all","Skip",B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);
		switch (myAlert->Go())
		{
			case 0:
				Close();
				kill_thread(m_MoveThread);
				break;
			case 1:
				m_SkipAllMoveError = true;
				break;
		}	
	}
	
	return result;
}

////////////////////////////////////////////////////////////////////////
int32 GenesisMoveWindow::GetFirstSelection(void)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	
	item = m_CustomListView->GetSelectedEntry(0);
	if (item)
		return m_CustomListView->IndexOf(item);
	else
		return 0;	
}

////////////////////////////////////////////////////////////////////////
bool GenesisMoveWindow::IsDirReadOnly(const char *destination)
////////////////////////////////////////////////////////////////////////
{
	struct stat statbuf;	
	BDirectory dir(destination);
	BVolume volume;
	
	if (dir.InitCheck()!=B_OK)
		return false;
	
	if (dir.GetStatFor(destination, &statbuf)!=B_OK)
		return false;
	
	volume.SetTo(statbuf.st_dev);
	if (volume.IsReadOnly())
		return true;
	
	return false;	// Not read only
}

////////////////////////////////////////////////////////////////////////
bool GenesisMoveWindow::IsRecursiveMove(const char *source, const char *destination)
////////////////////////////////////////////////////////////////////////
{
	BEntry src(source);
	BEntry dst(destination);
	
	if (src == dst)
		return true;
		
	while ((dst.GetParent(&dst)) == B_OK)
	{
		if (src == dst)
			return true;
	}

	return false;
}

