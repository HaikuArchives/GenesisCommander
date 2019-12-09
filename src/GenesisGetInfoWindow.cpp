/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#include "GenesisGetInfoWindow.h"
#include "GenesisIconView.h"
#include "GenesisWindow.h"
#include <stdio.h>
#include <View.h>
#include <Window.h>
#include <Button.h>
#include <StringView.h>
#include <Entry.h>
#include <String.h>
#include <StringList.h>
#include <Node.h>		// for icon
#include <NodeInfo.h>
#include <Beep.h>
#include <Box.h>
#include <Path.h>
#include <VolumeRoster.h>
#include <Directory.h>

////////////////////////////////////////////////////////////////////////
GenesisGetInfoWindow::GenesisGetInfoWindow(const char *dir, BStringList *files, BWindow *mainwindow) :
	BWindow(BRect(100,100,460,308), "File information...", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE);

	AddToSubset(mainwindow);
	
	// Main view
	m_View = new BView(Bounds(), "infoview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewColor(216, 216, 216, 0);
	AddChild(m_View);

	// Bottom View	
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);	
	
	// OK Button	
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;	
	BButton *OkButton = new BButton(rect,"ok","OK",new BMessage(BUTTON_MSG_OK),0,B_WILL_DRAW);
	BottomView->AddChild(OkButton);
	
	SetDefaultButton(OkButton);

	// Icon Box
	m_IconBox = new BBox(BRect(8,8,8+31+4,8+31+4),"iconbox",B_FOLLOW_NONE,B_WILL_DRAW,B_FANCY_BORDER);
	m_View->AddChild(m_IconBox);

	// Main view
	rect = m_IconBox->Bounds();
	rect.InsetBy(2,2);
	m_IconView = new IconView(rect, "iconview");
	m_IconBox->AddChild(m_IconView);	

	rect = m_View->Bounds();
	rect.left += 2;
	rect.right -= 2;
	rect.top = 52;
	rect.bottom = 53;
	BBox *Bevel_1 = new BBox(rect);
	m_View->AddChild(Bevel_1);

// -------------------------------------------------------------------------
	
	if (files->CountStrings() == 1){
		// File informations....
		BString filename;
		filename.SetToFormat("%s/%s", dir, files->StringAt(0).String());

		BEntry entry(filename);
		int entrytype;

		// Let's get its type...
		if (entry.IsDirectory()) entrytype = ET_DIRECTORY;
		else if (entry.IsSymLink()) entrytype = ET_SYMLINK;
		else entrytype = ET_FILE;

		switch (entrytype)
		{
			case ET_DIRECTORY:
				SetTitle("Folder information...");
				ExamineDirectory(filename);
				break;
			case ET_SYMLINK:
				SetTitle("Symbolic link information...");
				ExamineSymLink(filename);
				break;
			case ET_FILE:
				SetTitle("File information...");
				ExamineFile(filename);
				break;
		}
	}
	else{
		SetTitle("Files information...");
		ExamineMultipleFiles(dir, files);	
	}

	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_OK)));

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
GenesisGetInfoWindow::~GenesisGetInfoWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisGetInfoWindow::ExamineDirectory(const char* filename)
////////////////////////////////////////////////////////////////////////
{
	BEntry entry(filename);
	BNode node(filename);
	BNodeInfo nodeinfo(&node);

	BStringView *sv;
	BString text;
	
	char buf[B_FILE_NAME_LENGTH];

	// Let's get its icon...
	m_IconView->SetIcon(&nodeinfo);

	// Get filename...	
	entry.GetName(buf);
	text.SetTo(buf);
	sv = new BStringView(BRect(48,20,390,32),"filename",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get its path...
	BPath path;
	entry.GetPath(&path);
	text.SetTo("Full path: ");
	text += path.Path();
	sv = new BStringView(BRect(8,60,390,74),"filepath",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Type: Folder
	sv = new BStringView(BRect(8,80,390,94),"filetype","Type: Folder");
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get directory entry allocation size...
	struct stat statbuf;
	entry.GetStat(&statbuf);
	text.SetTo("Folder entry allocates ");
	text << statbuf.st_size;
	text += " bytes.";
	sv = new BStringView(BRect(8,100,390,114),"filesize",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);
}

////////////////////////////////////////////////////////////////////////
void GenesisGetInfoWindow::ExamineSymLink(const char* filename)
////////////////////////////////////////////////////////////////////////
{
	BEntry entry(filename);
	BEntry symlinkentry;
	entry_ref ref;
	BNode node;
	BNodeInfo nodeinfo; //(&node);
	
	BString text;
	BStringView *sv;
	
	char buf[B_FILE_NAME_LENGTH];

	entry.GetRef(&ref);
	symlinkentry.SetTo(&ref, true);

	node.SetTo(&symlinkentry);
	nodeinfo.SetTo(&node);	
	
	// Let's get its icon...
	m_IconView->SetIcon(&nodeinfo);

	// Get filename...	
	entry.GetName(buf);
	text.SetTo(buf);
	sv = new BStringView(BRect(48,20,390,32),"filename",text.String());
	sv->ResizeToPreferred();	
	m_View->AddChild(sv);
	
	// Get its path...
	BPath path;
	entry.GetPath(&path);
	text.SetTo("Full path: ");
	text += path.Path();
	sv = new BStringView(BRect(8,60,390,74),"filepath",text.String());
	sv->ResizeToPreferred();	
	m_View->AddChild(sv);

	// Type: Symbolic link
	sv = new BStringView(BRect(8,80,390,94),"filetype","Type: Symbolic link");
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get file size...
	struct stat statbuf;
	entry.GetStat(&statbuf);
	text.SetTo("File size: ");
	text << statbuf.st_size;
	text += " bytes";
	sv = new BStringView(BRect(8,100,390,114),"filesize",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get linked file/folder path...
	BPath linkpath;
	symlinkentry.GetPath(&linkpath);
	text.SetTo("Link: ");
	text += linkpath.Path();
	sv = new BStringView(BRect(8,120,390,134),"linkedfilepath",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get linked file size...
	symlinkentry.GetStat(&statbuf);
	text.SetTo("Linked file size: ");
	text << statbuf.st_size;
	text += " bytes";
	sv = new BStringView(BRect(8,140,390,154),"linkedfilesize",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);
	
}

////////////////////////////////////////////////////////////////////////
void GenesisGetInfoWindow::ExamineFile(const char* filename)
////////////////////////////////////////////////////////////////////////
{
	BEntry entry(filename);
	BNode node(filename);
	BNodeInfo nodeinfo(&node);

	BStringView *sv;
	BString text;

	char buf[B_FILE_NAME_LENGTH];

	// Let's get its icon...
	m_IconView->SetIcon(&nodeinfo);

	// Get filename...	
	entry.GetName(buf);
	text.SetTo(buf);
	sv = new BStringView(BRect(48,20,390,32),"filename",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get its path...
	BPath path;
	entry.GetPath(&path);
	text.SetTo("Full path: ");
	text += path.Path();
	sv = new BStringView(BRect(8,60,390,74),"filepath",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Type: Plain file
	sv = new BStringView(BRect(8,80,390,94),"filetype","Type: Plain file");
	sv->ResizeToPreferred();	
	m_View->AddChild(sv);

	// Get file size...
	struct stat statbuf;
	entry.GetStat(&statbuf);
	text.SetTo("File size: ");
	text << statbuf.st_size;
	text += " bytes";
	sv = new BStringView(BRect(8,100,390,114),"filesize",text.String());
	sv->ResizeToPreferred();	
	m_View->AddChild(sv);
}

////////////////////////////////////////////////////////////////////////
void GenesisGetInfoWindow::ExamineMultipleFiles(const char *dir, const BStringList *filesList)
////////////////////////////////////////////////////////////////////////
{
	BString filepath;
	BEntry entry;
	struct stat statbuf;

	BStringView *sv;
	BString text;

	int files = 0;
	int directories = 0;
	int symlinks = 0;
	off_t totalSize = 0;

	for (int i = 0; i < filesList->CountStrings(); i++)
	{
		filepath.SetToFormat("%s/%s", dir, filesList->StringAt(i).String());
		entry.SetTo(filepath.String());

		if (entry.IsDirectory())
			directories++;
		else if (entry.IsSymLink())
			symlinks++;
		else
		{
			files++;
			entry.GetStat(&statbuf);
			totalSize += statbuf.st_size;
		}
		entry.Unset();
	}
	
	m_IconBox->Hide();

	// label
	text.SetToFormat("%d files selected in %s", filesList->CountStrings(), dir);
	sv = new BStringView(BRect(8,20,390,32),"label",text.String());
	
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// file types
	text.SetToFormat("Folders: %d", directories);
	sv = new BStringView(BRect(8,60,390,74),"directories",text);
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	text.SetToFormat("Plain files: %d", files);
	sv = new BStringView(BRect(8,80,390,94),"regularf",text);
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	text.SetToFormat("Symbolic links: %d", symlinks);
	sv = new BStringView(BRect(8,100,390,114),"symlinks",text);
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// files size
	if (files > 0)
	{
		text.SetTo("Files size: ");
		text << totalSize << " bytes";
		sv = new BStringView(BRect(8,140,390,154),"filesize",text.String());
		sv->ResizeToPreferred();
		m_View->AddChild(sv);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisGetInfoWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case BUTTON_MSG_OK:
			Close();
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


// ***************************************************************************


////////////////////////////////////////////////////////////////////////
GenesisGetDiskInfoWindow::GenesisGetDiskInfoWindow(CustomListItem *item, BWindow *mainwindow) :
	BWindow(BRect(100,100,460,348), "Disk information...", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE);

	AddToSubset(mainwindow);

//	SetType(B_MODAL_WINDOW);
//	SetFlags(B_NOT_RESIZABLE);
	
	// Main view
	m_View = new BView(Bounds(), "infoview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewColor(216, 216, 216, 0);
	AddChild(m_View);

	// Bottom View	
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);	
	
	// OK Button	
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;	
	BButton *OkButton = new BButton(rect,"ok","OK",new BMessage(BUTTON_MSG_OK),0,B_WILL_DRAW);
	BottomView->AddChild(OkButton);
	
	SetDefaultButton(OkButton);

	// Icon Box
	BBox *IconBox = new BBox(BRect(8,8,8+31+4,8+31+4),"iconbox",B_FOLLOW_NONE,B_WILL_DRAW,B_FANCY_BORDER);
	m_View->AddChild(IconBox);

	// Main view
	rect = IconBox->Bounds();
	rect.InsetBy(2,2);
	m_IconView = new IconView(rect, "iconview");
	IconBox->AddChild(m_IconView);

	rect = m_View->Bounds();
	rect.left += 2;
	rect.right -= 2;
	rect.top = 52;
	rect.bottom = 53;
	BBox *Bevel_1 = new BBox(rect);
	m_View->AddChild(Bevel_1);
	
	// Get our drive...
	BVolumeRoster *vr = new BVolumeRoster();
	if (vr)
	{
		BVolume v;
		bool found = false;
	
		while (vr->GetNextVolume(&v)==B_NO_ERROR)
		{
			if (v.Device()==item->m_DeviceID)
			{
				ExamineDevice(&v);
				found = true;
			}
		}

		delete vr;
	}
	
	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_OK)));
	
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
GenesisGetDiskInfoWindow::~GenesisGetDiskInfoWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisGetDiskInfoWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case BUTTON_MSG_OK:
			Close();
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisGetDiskInfoWindow::ExamineDevice(BVolume *v)
////////////////////////////////////////////////////////////////////////
{
	char buf[B_FILE_NAME_LENGTH];	// drive-hoz jo ez?
	off_t capacity,freebytes;

	BStringView *sv;
	BString text;

	// Let's get its icon...
	m_IconView->SetIcon(v);
	
	// Get filename...	
	v->GetName(buf);
	text.SetTo(buf);
	sv = new BStringView(BRect(48,20,390,32),"drivename",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get its path...
	BDirectory dir;
	BEntry entry;
	BPath path;
	v->GetRootDirectory(&dir);
	dir.GetEntry(&entry);
	entry.GetPath(&path);
	text.SetTo("Root directory: ");
	text += path.Path();
	sv = new BStringView(BRect(8,60,390,74),"driverootpath",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);	

	// Get drive capacity...
	capacity = v->Capacity();
	if (capacity>=(1024*1024*1024))
		sprintf(buf,"%.02f GB",capacity/(1024.0*1024.0*1024.0));
	else 
		sprintf(buf,"%.02f MB",capacity/(1024.0*1024.0));
	text.SetTo("Drive capacity: ");
	text += buf;
	sv = new BStringView(BRect(8,80,390,94),"drivesize",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	// Get free space...
	freebytes = v->FreeBytes();
	if (freebytes>=(1024*1024*1024))
		sprintf(buf,"%.02f GB",freebytes/(1024.0*1024.0*1024.0));
	else if (freebytes>=(1024*1024))
		sprintf(buf,"%.02f MB",freebytes/(1024.0*1024.0));
	else if (freebytes>=1024)
		sprintf(buf,"%.02f KB",freebytes/1024.0);
	else
		sprintf(buf,"%d byte%s",(int)freebytes,freebytes==1?"":"s");
	text.SetTo("Free space: ");
	text += buf;
	sv = new BStringView(BRect(8,100,390,114),"drivesize",text.String());
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	if (v->IsReadOnly())
		sv = new BStringView(BRect(8,120,390,134),"drivesize","ReadOnly: Yes");
	else
		sv = new BStringView(BRect(8,120,390,134),"drivesize","ReadOnly: No");
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	if (v->IsRemovable())
		sv = new BStringView(BRect(8,140,390,154),"drivesize","Removable: Yes");
	else
		sv = new BStringView(BRect(8,140,390,154),"drivesize","Removable: No");
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	if (v->IsPersistent())
		sv = new BStringView(BRect(8,160,390,174),"drivesize","Persistent: Yes");
	else
		sv = new BStringView(BRect(8,160,390,174),"drivesize","Persistent: No");
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	if (v->IsShared())
		sv = new BStringView(BRect(8,180,390,194),"drivesize","Shared: Yes");
	else
		sv = new BStringView(BRect(8,180,390,194),"drivesize","Shared: No");
	sv->ResizeToPreferred();
	m_View->AddChild(sv);

	m_PieView = new PieView(BRect(160,84,340,192),"pieview", capacity, freebytes);
	m_View->AddChild(m_PieView);
}

////////////////////////////////////////////////////////////////////////
PieView::PieView(BRect rect, const char *name, off_t capacity, off_t free)
	   	   : BView( rect, name , 0, B_WILL_DRAW )
////////////////////////////////////////////////////////////////////////
{
	SetViewColor(216, 216, 216, 0);
	
	m_Capacity = capacity;
	m_Free = free;
}

////////////////////////////////////////////////////////////////////////
PieView::~PieView()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void PieView::Draw(BRect r)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	float usedpercent = ((float)(m_Capacity - m_Free) / m_Capacity) * 100.0f;

	rect = Bounds();
	rect.bottom-=16;

	SetHighColor(180,190,200);
	FillArc(rect, 0.0f, 360.0 );

	SetHighColor(160,170,180);
	StrokeArc(rect, 0.0f, 360.0 );

	SetHighColor(140,150,160);
	FillArc(rect, 0.0f, 360.0/100.0*usedpercent );

	SetHighColor(100,110,120);
	StrokeArc(rect, 0.0f, 360.0/100.0*usedpercent );
	
	// Free space
	rect = Bounds();
	rect.left = 0;
	rect.right = 10;
	rect.top = rect.bottom-10;

	SetHighColor(180,190,200);
	FillRect(rect);
	SetHighColor(0,0,0);
	StrokeRect(rect);
	
	SetHighColor(0,0,0);
	SetLowColor(216, 216, 216, 0);
	DrawString("- Free space",BPoint(rect.right+4,rect.top+9));
	
	// Used space
	rect = Bounds();
	rect.top = rect.bottom-10;
	SetHighColor(0,0,0);
	SetLowColor(216, 216, 216, 0);
	DrawString("- Used space",BPoint(rect.right-StringWidth("- Used space"),rect.top+9));

	rect.right = rect.right-StringWidth("- Used space")-4;
	rect.left = rect.right-10;

	SetHighColor(140,150,160);
	FillRect(rect);
	SetHighColor(0,0,0);
	StrokeRect(rect);
}

