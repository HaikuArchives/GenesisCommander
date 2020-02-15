/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019-2020, Ondrej Čerman
 */

#include "GenesisCopyWindow.h"
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
GenesisCopyWindow::GenesisCopyWindow(CustomListView *list, PanelView *destpanel, const char *destination, BLooper* looper, BWindow *mainwindow) :
	BWindow(BRect(0,0,320,140), "Copy...", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	m_CustomListView = list;
	m_DestPanel = destpanel;
	m_DestPath.SetTo(destination);
	m_Looper = looper;
	m_Window = mainwindow;

	m_SingleCopy = false;
	m_Paused = false;
	m_SkipAllCopyError = false;
	m_OverwriteAll = false;
	m_SkipSymLinkCreationError = false;
	m_PossiblyMultipleFiles = true;

	// After the delete process we have to select an item if no item selected...
	m_Selection = GetFirstSelection();

	// First we have to remove the parent selection if selected...
//	RemoveParentSelection();

	m_FileCount = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);

	if (m_FileCount == 1)
		m_SingleCopy = true;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE);

	AddToSubset(mainwindow);

	m_View = new BView(Bounds(), "copyview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_View);

	// Bottom View
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_BOTTOM, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);
/*
	// More checkbox
	rect = BottomView->Bounds();
	rect.top = rect.bottom-30;
	rect.bottom = rect.bottom-14;
	rect.left = 20;
	rect.right = rect.left+40;
	m_MoreCB = new BCheckBox(rect,"more","Show more",new BMessage(COPY_MORE), B_FOLLOW_BOTTOM, B_WILL_DRAW);
	m_MoreCB->ResizeToPreferred();
	BottomView->AddChild(m_MoreCB);
*/
	// OK Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;
	m_CopyButton = new BButton(rect,"copy","Copy",new BMessage(BUTTON_MSG_COPY),B_FOLLOW_BOTTOM,B_WILL_DRAW);
	BottomView->AddChild(m_CopyButton);

	//Cancel Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;
	m_CancelButton = new BButton(rect,"cancel","Cancel",new BMessage(BUTTON_MSG_CANCELCOPY),B_FOLLOW_BOTTOM,B_WILL_DRAW);
	BottomView->AddChild(m_CancelButton);

	SetDefaultButton(m_CopyButton);

	// Info string
	rect = Bounds();
	rect.left += 24;
	rect.right -= 24;
	if (m_SingleCopy)
		rect.top += 8;
	else
		rect.top += 16;
	rect.bottom = rect.top+20;
	m_Label = new BStringView(rect,"filename","Copy");
	m_Label->SetAlignment(B_ALIGN_CENTER);
	m_Label->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_Label);

	// Edit field
	rect = BottomView->Bounds();
	if (m_SingleCopy)
		rect.top = rect.top+36;
	else
		rect.top = rect.top+56;
	rect.bottom = rect.top+32;
	rect.left += 20;
	rect.right -= 20;
	m_DirName = new BTextControl( rect, "destname", "to", "", NULL );
	m_DirName->SetDivider(m_View->StringWidth("to")+4);
	m_DirName->SetModificationMessage(new BMessage(COPYNAME_CHANGED));
	m_DirName->SetText(m_DestPath.String());
	m_View->AddChild(m_DirName);

	// "as" field
	if (m_SingleCopy)
	{
		rect = BottomView->Bounds();
		rect.top = rect.top+64;
		rect.bottom = rect.top+32;
		rect.left += 20;
		rect.right -= 20;

		m_FileAsName = new BTextControl( rect, "destfilename", "as", "", NULL );
		m_FileAsName->SetDivider(m_View->StringWidth("to")+4);
		m_FileAsName->SetModificationMessage(new BMessage(COPYNAME_CHANGED));

		CustomListItem *file = (CustomListItem *)m_CustomListView->GetSelectedEntry(0);
		if (file->m_Type == FT_PARENT)
			file = (CustomListItem *)m_CustomListView->GetSelectedEntry(1);

		if (file->m_Type != FT_DIRECTORY)
			m_PossiblyMultipleFiles = false;

		m_FileAsName->SetText(file->m_FileName.String());
		m_View->AddChild(m_FileAsName);
	}

	m_DirName->MakeFocus(true);

	// Ctrl + Q closes the window...
	AddShortcut('Q', 0, new BMessage(BUTTON_MSG_CANCELCOPY));

	// Set copy label...
	if (m_SingleCopy)
	{
		BString text;
		text.SetTo("Copy '");
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
		text.SetTo("Copy ");
		text << m_FileCount << " files";

		m_Label->SetText(text.String());
	}

	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCELCOPY)));

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
GenesisCopyWindow::~GenesisCopyWindow()
////////////////////////////////////////////////////////////////////////
{
	if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==0)
	{
		if (m_Selection > (m_CustomListView->IndexOf(m_CustomListView->LastItem())))
			m_Selection = m_CustomListView->IndexOf(m_CustomListView->LastItem());

		m_CustomListView->Select(m_Selection, false);	// false -> remove previously selected item(s)...
	}
/*
	if (m_DestPanel && strcmp(m_DestPath.String(), m_DestPanel->m_Path.String())==0)
	{
		m_Window->Lock();
		m_DestPanel->Reload();
		m_Window->Unlock();
	}
	else //if (strcmp(m_DestPath.String(), ((PanelView *)m_CustomListView->m_PV)->m_Path.String())==0)
	{
		m_Window->Lock();
		((PanelView *)m_CustomListView->m_PV)->Reload();
		m_Window->Unlock();
	}
*/
}

////////////////////////////////////////////////////////////////////////
void GenesisCopyWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
/*		case COPY_MORE:
			if (m_MoreCB->Value())
				ResizeTo(320,180);
			else
				ResizeTo(320,140);
			break;
*/
		case COPYNAME_CHANGED:
			if (m_SingleCopy)	// Csak egy file van es akkor mind a ket mezot figyelni kell...
			{
				if (strlen(m_DirName->Text())>0 && strlen(m_FileAsName->Text())>0)
					m_CopyButton->SetEnabled(true);
				else
					m_CopyButton->SetEnabled(false);
			}
			else
			{
				if (strlen(m_DirName->Text())>0)
					m_CopyButton->SetEnabled(true);
				else
					m_CopyButton->SetEnabled(false);
			}
			break;
		case BUTTON_MSG_CANCELCOPY:
			if (find_thread("CopyThread")!=B_NAME_NOT_FOUND)	// Ha mar megy a masolas...
			{
				BAlert *myAlert = new BAlert("Copy","Do you really want to abort?","No","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
				myAlert->SetShortcut(0, B_ESCAPE);
				if (myAlert->Go()==1)
				{
					kill_thread(m_CopyThread);
					Close();
				}
			}
			else
				Close();
			break;
		case BUTTON_MSG_COPY:
			PrepareCopy();
			break;
		case BUTTON_MSG_PAUSECOPY:
			if (m_Paused)
			{
				if (resume_thread(m_CopyThread)==B_OK)
				{
					m_PauseButton->SetLabel("Pause");
					m_Paused = false;
				}
			}
			else
			{
				if (suspend_thread(m_CopyThread)==B_OK)
				{
					m_PauseButton->SetLabel("Resume");
					m_Paused = true;
				}
			}
			break;
		case BUTTON_MSG_ABORTCOPY:
			{
				BAlert *myAlert = new BAlert("Copy","Do you really want to abort?","No","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
				myAlert->SetShortcut(0, B_ESCAPE);
				if (myAlert->Go()==1)
				{
					kill_thread(m_CopyThread);
					Close();
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisCopyWindow::RemoveParentSelection()
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
void GenesisCopyWindow::Go(void)
////////////////////////////////////////////////////////////////////////
{
	Show();
}

////////////////////////////////////////////////////////////////////////
int32 SingleCopyThreadFunc(void *data)
////////////////////////////////////////////////////////////////////////
{
	GenesisCopyWindow *d;
	BString filename;
	BString text;
	CustomListItem *item;

	d = (GenesisCopyWindow *)data;

	item = (CustomListItem *)d->m_CustomListView->GetSelectedEntry(0);
	if (item)
	{
		filename.SetTo(item->m_FileName); // = (BString *)d->m_FileList->ItemAt(i);
		text.SetTo("");
		text << "Copying '" << filename.String() << "' as '" << d->m_DestFileName.String() << "'";
		d->Lock();
		d->m_FileBar->SetText(text.String());
		d->Unlock();

		filename.SetTo(item->m_FilePath);
		filename += '/';
		filename += item->m_FileName;
		d->Copy(filename.String(), d->m_DestPath.String(), d->m_DestFileName.String());

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
int32 MultiCopyThreadFunc(void *data)
////////////////////////////////////////////////////////////////////////
{
	GenesisCopyWindow *d;
	BString filename;
	BString text;
	CustomListItem *item;

	d = (GenesisCopyWindow *)data;

	for(int i=0;i<(d->m_FileCount);i++)
	{
		item = (CustomListItem *)d->m_CustomListView->GetSelectedEntry(0);
		if (item)
		{
			filename.SetTo(item->m_FileName); // = (BString *)d->m_FileList->ItemAt(i);
			text.SetTo("");
			text << "Copying '" << filename.String() << "'";
			d->Lock();
			d->m_FileBar->SetText(text.String());
			d->Unlock();

			filename.SetTo(item->m_FilePath);
			filename += '/';
			filename += item->m_FileName;
			d->Copy(filename.String(), d->m_DestPath.String());

			d->Lock();
			d->m_ProgressBar->Update(1);
			d->Unlock();

			d->m_Window->Lock();
			d->m_CustomListView->Deselect(d->m_CustomListView->IndexOf(item));
			d->m_CustomListView->InvalidateItem(d->m_CustomListView->IndexOf(item));
			d->m_Window->Unlock();
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
void GenesisCopyWindow::PrepareCopy(void)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	rgb_color BarColor;
	BarColor.red = 180;
	BarColor.green = 190;
	BarColor.blue = 200;

	m_DestPath.SetTo(m_DirName->Text());

	if (m_SingleCopy)
		m_DestFileName.SetTo(m_FileAsName->Text());

	if (IsDirReadOnly(m_DestPath.String()))
	{
		BAlert *myAlert = new BAlert("Copy", "Cannot copy to a write protected volume.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);
		myAlert->Go();
		return;
	}

	BEntry dest(m_DestPath.String());

	if (dest.InitCheck()!=B_OK)
	{
		BAlert *myAlert = new BAlert("Copy","Cannot initialize destination entry.","OK", NULL, NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->Go();
		return;
	}

	if (!dest.Exists())
	{
		BAlert *myAlert = new BAlert("Copy","Destination path does not exist.","OK", NULL, NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->Go();
		return;
	}

	if (!dest.IsDirectory())
	{
		BAlert *myAlert = new BAlert("Copy","Destination path is not a folder.","OK", NULL, NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->Go();
		return;
	}

	BAutolock autolocker(this);

	m_AbortButton = m_CopyButton;
	m_AbortButton->SetMessage(new BMessage(BUTTON_MSG_ABORTCOPY));
	m_PauseButton = m_CancelButton;
	m_PauseButton->SetMessage(new BMessage(BUTTON_MSG_PAUSECOPY));

	m_DirName->RemoveSelf();
	m_Label->RemoveSelf();				//	m_Label->MoveBy(0,40);

	if (m_SingleCopy)
		m_FileAsName->RemoveSelf();

	SetTitle("Copy in progress...");

	// Add ProgressBar
	if (!m_SingleCopy)
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
	if (m_SingleCopy)
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

	if (m_SingleCopy)
		m_CopyThread = spawn_thread(SingleCopyThreadFunc, "CopyThread", B_NORMAL_PRIORITY, (void *)this);
	else
		m_CopyThread = spawn_thread(MultiCopyThreadFunc, "CopyThread", B_NORMAL_PRIORITY, (void *)this);
	resume_thread(m_CopyThread);
}

////////////////////////////////////////////////////////////////////////
void GenesisCopyWindow::Copy(const char *filename, const char *destination, const char *destfilename)
////////////////////////////////////////////////////////////////////////
{
	BEntry sourcefile(filename);
	BString text;

	if (sourcefile.InitCheck()==B_OK)
	{
/*
		BString text;

		text.SetTo("");
		text << filename << "\n" << destination;

		BAlert *myAlert = new BAlert("Copy debug",text.String(),"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		myAlert->Go();
*/

		if (sourcefile.IsDirectory())
		{
			if (IsRecursiveCopy(filename, destination))
			{
				BString text;
				text << "Recursive copy not allowed.\nPlease check the destination folder.";

				BAlert *myAlert = new BAlert("Copy",text.String(),"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
				myAlert->Go();
				Close();
				kill_thread(m_CopyThread);
			}

			CopyDirectory(filename, destination, destfilename);
		}
		else if (sourcefile.IsSymLink())
			CopyLink(filename,destination, destfilename);
		else
			CopyFile(filename, destination, destfilename);
	}
	else if (!m_SkipAllCopyError)
	{
		text << "Error while initializing file:\n\n" << filename;
		switch (CopySkipAlert(text.String()))
		{
			case A_SKIP_ABORT:
				Close();
				kill_thread(m_CopyThread);
				break;
			case A_SKIP_ALL:
				m_SkipAllCopyError = true;
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////
bool GenesisCopyWindow::CopyFile(const char *filename, const char *destination, const char *destfilename)
////////////////////////////////////////////////////////////////////////
{
	char name[B_FILE_NAME_LENGTH];
	BString destname;
	BEntry srcentry(filename);
	BEntry dstentry(destination);
	struct stat statbuf;
	ssize_t len;

	srcentry.GetName(name);

	destname.SetTo(destination);
	destname += "/";
	if (destfilename)
		destname += destfilename;
	else
		destname += name;

	BEntry dstfileentry(destname.String());
	if (dstfileentry.InitCheck()!=B_OK)
		return false;
	if (dstfileentry.Exists() && !m_OverwriteAll)
	{
		BString text;

		if (dstfileentry.IsDirectory())
		{
			if (!m_SkipAllCopyError) {
				text << "Directory '" << name << "' cannot be overwritten with a file.";
				switch (CopySkipAlert(text.String()))
				{
					case A_SKIP_ABORT:
						Close();
						kill_thread(m_CopyThread);
						break;
					case A_SKIP_ALL:
						m_SkipAllCopyError = true;
						break;
				}
			}
			return false;
		}
		else{
			if (!m_OverwriteAll) {
				text << "File '" << name << "' already exists. Do you want to overwrite it?";
				switch (CopyOverwriteAlert(text.String()))
				{
					case A_OVERWR_ABORT:
						Close();
						kill_thread(m_CopyThread);
						break;
					case A_OVERWR_ALL:
						m_OverwriteAll = true;
						break;
				}

				if (dstfileentry.IsSymLink()){
					dstfileentry.Remove();
				}
			}
		}
	}

	BFile srcfile(filename, B_READ_ONLY);
	BFile dstfile(destname.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);

	if (srcentry.InitCheck()!=B_OK)
		return false;

	if (dstentry.InitCheck()!=B_OK)
		return false;

	if (!srcentry.Exists())
		return false;

	if (!dstentry.Exists())
	{
		return false;
	}

	if (srcentry.GetStat(&statbuf)!=B_OK)
	{
		return false;
	}

	unsigned char *buf = new unsigned char[statbuf.st_blksize];
	if (!buf)
	{
		return false;
	}

	Lock();
	m_FileBar->Update(-m_FileBar->CurrentValue());	// Reset to 0.0
	m_FileBar->SetMaxValue(statbuf.st_size);
	m_FileBar->SetTrailingText(name);
	Unlock();

	while (true)
	{
		len = srcfile.Read(buf, statbuf.st_blksize);
		if (len>0)
		{
			dstfile.Write(buf, len);
			Lock();
			m_FileBar->Update(len);
			Unlock();
		}
		else if (len<0) // error
		{
			delete [] buf;
			return false;
		}
		else	// No more bytes to copy, we are done...
			break;
	}

	dstfile.SetPermissions(statbuf.st_mode);
	dstfile.SetOwner(statbuf.st_uid);
	dstfile.SetGroup(statbuf.st_gid);
	dstfile.SetModificationTime(statbuf.st_mtime);
	dstfile.SetCreationTime(statbuf.st_crtime);

	delete [] buf;

	// Copy attributes...
	CopyAttr(filename, destname.String());

	return true;
}

////////////////////////////////////////////////////////////////////////
void GenesisCopyWindow::CopyDirectory(const char *dirname, const char *destination, const char *destdirname)
////////////////////////////////////////////////////////////////////////
{
	BEntry srcentry(dirname);
	BEntry dstentry;
	char name[B_FILE_NAME_LENGTH];
	BString fulldestdir;

	if (srcentry.InitCheck()!=B_OK)
		return;

	if (!srcentry.Exists())
		return;

	srcentry.GetName(name);

	fulldestdir.SetTo(destination);
	if (destdirname)
		fulldestdir << "/" << destdirname;
	else
		fulldestdir << "/" << name;

	dstentry.SetTo(fulldestdir.String());

	if (dstentry.InitCheck()!=B_OK)
		return;

	if (!dstentry.Exists())
	{
		if (create_directory(fulldestdir.String(), 0777)!=B_OK)		// TODO: jo a 0777?
			return;
	}

	BDirectory dir;

	dir.SetTo(dirname);
	if (dir.InitCheck()==B_OK)
	{
		BEntry entry;

		if (dir.GetEntry(&entry)==B_OK)
		{
			while (dir.GetNextEntry(&entry)==B_OK)
			{
				entry.GetName(name);

				if (entry.IsDirectory())
				{
					BString fullname;

					fullname.SetTo(dirname);
					fullname << "/" << name;
					CopyDirectory(fullname.String(), fulldestdir.String());
				}
				else if (entry.IsSymLink())
				{
					BString fullname;

					fullname.SetTo(dirname);
					fullname << "/" << name;
					CopyLink(fullname.String(), fulldestdir.String());
				}
				else
				{
					BString fullname;

					fullname.SetTo(dirname);
					fullname << "/" << name;
					CopyFile(fullname.String(), fulldestdir.String());
				}
			}
		}
	}

	// Copy attributes...
	CopyAttr(dirname, fulldestdir.String());
}

////////////////////////////////////////////////////////////////////////
bool GenesisCopyWindow::CopyLink(const char *linkname, const char *destination, const char *destfilename)
////////////////////////////////////////////////////////////////////////
{
	BSymLink srclink;
	BSymLink dstlink;
	BDirectory dstdir;
	BEntry srcentry;
	BEntry symlinkentry;
	BPath LinkPath;
	char name[B_FILE_NAME_LENGTH];
	struct stat statbuf;
	entry_ref ref;

	srcentry.SetTo(linkname);
	srcentry.GetName(name);
	srcentry.GetRef(&ref);
	symlinkentry.SetTo(&ref, true);
	symlinkentry.GetPath(&LinkPath);

	if (destfilename)
		sprintf(name,"%s",destfilename);

	if (srcentry.GetStat(&statbuf)!=B_OK)
		return false;

	dstdir.SetTo(destination);

	if (dstdir.InitCheck()!=B_OK)
		return false;

	Lock();
	m_FileBar->Update(-m_FileBar->CurrentValue());	// Reset to 0.0
	m_FileBar->SetMaxValue(1);
	m_FileBar->SetTrailingText(name);
	Unlock();

	if (dstdir.CreateSymLink(name, LinkPath.Path(), &dstlink)!=B_OK && !m_SkipSymLinkCreationError)
	{
		BString text;
		text << "Cannot create '" << name << "' symbolic link in '" << LinkPath.Path() << "'";

		switch (CopySkipAlert(text.String()))
		{
			case A_SKIP_ABORT:
				Close();
				kill_thread(m_CopyThread);
				break;
			case A_SKIP_ALL:
				m_SkipSymLinkCreationError = true;
				break;
		}

		return false;
	}

	Lock();
	m_FileBar->Update(1);
	Unlock();

	dstlink.SetPermissions(statbuf.st_mode);
	dstlink.SetOwner(statbuf.st_uid);
	dstlink.SetGroup(statbuf.st_gid);
	dstlink.SetModificationTime(statbuf.st_mtime);
	dstlink.SetCreationTime(statbuf.st_crtime);

	// Copy attributes...
	BString destlinkname;
	destlinkname.SetTo("");
	destlinkname << destination << "/" << name;
	CopyAttr(linkname, destlinkname.String());

	return true;
}

////////////////////////////////////////////////////////////////////////
bool GenesisCopyWindow::CopyAttr(const char *srcfilename, const char *dstfilename)
////////////////////////////////////////////////////////////////////////
{
	BNode srcnode(srcfilename);
	BNode dstnode(dstfilename);
	char attrname[B_ATTR_NAME_LENGTH];
	attr_info attrinfo;
	ssize_t len = 0;	// ennyit olvasott a ReadAttr()
	unsigned char *buf;

	while (srcnode.GetNextAttrName(attrname) == B_OK)
	{
		if (srcnode.GetAttrInfo(attrname, &attrinfo) != B_OK)
			continue;	// skip current attr

		buf = new unsigned char[attrinfo.size];
		if (buf)
		{
			len = srcnode.ReadAttr(attrname, attrinfo.type, 0, buf, attrinfo.size);

			if (len>0)
				dstnode.WriteAttr(attrname, attrinfo.type, 0, buf, attrinfo.size);

			delete [] buf;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////
int32 GenesisCopyWindow::GetFirstSelection(void)
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
bool GenesisCopyWindow::IsDirReadOnly(const char *destination)
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
bool GenesisCopyWindow::IsRecursiveCopy(const char *source, const char *destination)
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

////////////////////////////////////////////////////////////////////////
ALERT_SKIP_OPTS GenesisCopyWindow::CopySkipAlert(const char* text)
////////////////////////////////////////////////////////////////////////
{
	BAlert *alert = new BAlert("Copy", text, "Abort", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

	if (m_PossiblyMultipleFiles)
		alert->AddButton("Skip all");

	alert->AddButton("Skip");
	alert->SetShortcut(0, B_ESCAPE);

	switch (alert->Go())
	{
		case 0:
			return A_SKIP_ABORT;
		case 1:
			return m_PossiblyMultipleFiles ? A_SKIP_ALL: A_SKIP_1;
		default:
			return A_SKIP_1;
	}
}

////////////////////////////////////////////////////////////////////////
ALERT_OVERWR_OPTS GenesisCopyWindow::CopyOverwriteAlert(const char* text)
////////////////////////////////////////////////////////////////////////
{
	BAlert *alert = new BAlert("Copy", text, "Abort", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

	if (m_PossiblyMultipleFiles)
		alert->AddButton("Overwrite all");

	alert->AddButton("Overwrite");
	alert->SetShortcut(0, B_ESCAPE);

	switch (alert->Go())
	{
		case 0:
			return A_OVERWR_ABORT;
		case 1:
			return m_PossiblyMultipleFiles ? A_OVERWR_ALL: A_OVERWR_1;
		default:
			return A_OVERWR_1;
	}
}
