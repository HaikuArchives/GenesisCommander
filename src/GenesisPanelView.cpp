// 
//	GenesisPanelView.cpp
//
//	This file contains the main functions of a file panel.
//

#include "GenesisApp.h"
#include "GenesisPanelView.h"
#include "GenesisWindow.h"
#include "GenesisCustomListItem.h"
#include "GenesisGetInfoWindow.h"
#include "GenesisViewWindow.h"
#include "GenesisMakeDirWindow.h"
#include "GenesisDeleteWindow.h"
#include "GenesisCopyWindow.h"
#include "GenesisRenameWindow.h"
#include "GenesisMoveWindow.h"
#include "GenesisSeek.h"
#include "Language.h"

#include <stdio.h>
#include <stdlib.h>
#include <Roster.h>
#include <View.h>
#include <Alert.h>
#include <Cursor.h>
#include <Box.h>
#include <Beep.h>
#include <VolumeRoster.h>
#include <Volume.h>
#include <Beep.h>
#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <Node.h>
#include <File.h>
#include <SymLink.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OS.h>
#include <StorageDefs.h>
#include <Statable.h>
#include <FindDirectory.h>
#include <Resources.h>
#include <NodeMonitor.h>

////////////////////////////////////////////////////////////////////////
PanelView::PanelView(BRect rect, char *name)
	   	   : BView( rect, name , 0, B_WILL_DRAW )
////////////////////////////////////////////////////////////////////////
{
	BPath dirPath;			// To query some paths...
	BString tempstring;		// Ebben lehet kotoraszni a replace-eleshez

	// Default view color...
	SetViewColor(216, 216, 216, 0);

	// Default settings...
	m_Setting_ShowIcons = true;
	m_Setting_ShowFileSize = true;
	m_Setting_ShowFileDate = false;

	m_MonitoringPath.SetTo("");	// Empty string...

	// Resources
	LoadResources();

	// Main Box	
	m_Box = new BBox(Bounds(),"box",B_FOLLOW_ALL,B_WILL_DRAW,B_FANCY_BORDER);
	AddChild(m_Box);
	
	// Path
	m_PathMenu = new BMenu("cd",B_ITEMS_IN_COLUMN);

	m_CD_Parent = new BMenuItem(LANGS("CD_PARENT"),new BMessage(PATH_MSG_CD_PARENT),0,0);
	m_PathMenu->AddItem(m_CD_Parent);

	m_PathMenu->AddSeparatorItem();

	m_CD_Root = new BMenuItem(LANGS("CD_ROOT"),new BMessage(PATH_MSG_CD_ROOT),0,0);
	m_PathMenu->AddItem(m_CD_Root);

	find_directory(B_USER_DIRECTORY, &dirPath, true);
	tempstring = LANG("CD_HOME");
	tempstring.ReplaceAll("<DIR>", dirPath.Path());
	m_CD_Home = new BMenuItem(tempstring.String(),new BMessage(PATH_MSG_CD_HOME),0,0);
	m_PathMenu->AddItem(m_CD_Home);
	
	find_directory(B_DESKTOP_DIRECTORY, &dirPath, true);
	tempstring = LANG("CD_DESKTOP");
	tempstring.ReplaceAll("<DIR>", dirPath.Path());
	m_CD_Desktop = new BMenuItem(tempstring.String(),new BMessage(PATH_MSG_CD_DESKTOP),0,0);
	m_PathMenu->AddItem(m_CD_Desktop);

	m_PathMenu->AddSeparatorItem();
	m_CD_Disks = new BMenuItem(LANGS("CD_DISKS"),new BMessage(PATH_MSG_CD_DISKS),0,0);
	m_PathMenu->AddItem(m_CD_Disks);
		
	// CD BMenuField in the upper left area...
	m_PathField = new BMenuField(BRect(3,2,20,8*3),"cd","",m_PathMenu, false, 0, B_WILL_DRAW);
	m_PathField->ResizeToPreferred();
	m_PathField->SetDivider(0);
	m_Box->AddChild(m_PathField);

	// Menu in the top right corner...

	m_PanelMenu = new BMenu("",B_ITEMS_IN_COLUMN);

	m_PanelMenu_Find = new BMenuItem(LANGS("PANELMENU_FIND"),new BMessage(PANELMENU_MSG_FIND),0,0);
	m_PanelMenu_Find->SetEnabled(false);
	m_PanelMenu->AddItem(m_PanelMenu_Find);

	m_PanelMenu->AddSeparatorItem();

	m_PanelMenu_ShowIcons = new BMenuItem(LANGS("PANELMENU_SHOWICONS"),new BMessage(PANELMENU_MSG_SHOWICONS),0,0);
	if (m_Setting_ShowIcons) 
		m_PanelMenu_ShowIcons->SetMarked(true);
	else
		m_PanelMenu_ShowIcons->SetMarked(false);
	m_PanelMenu->AddItem(m_PanelMenu_ShowIcons);
	
	// Panel menu BMenuField in the upper right area...
	m_PanelMenuField = new BMenuField(BRect(Bounds().right-24,2,Bounds().right-4,8*3),"Menu","",m_PanelMenu, false, B_FOLLOW_TOP | B_FOLLOW_RIGHT, B_WILL_DRAW);
	m_PanelMenuField->ResizeToPreferred();
	m_PanelMenuField->SetDivider(0);
	m_Box->AddChild(m_PanelMenuField);

	// Path text in the upper area...
	m_PathStringView = new BStringView(BRect(38,2,280,20),"path","");
	m_PathStringView->SetAlignment(B_ALIGN_CENTER);
	m_Box->AddChild(m_PathStringView);
	
	m_Bevel_1 = new BBox(BRect(2,26,180,27), "bevel1");
	m_Box->AddChild(m_Bevel_1);

	m_Bevel_2 = new BBox(BRect(2,26,180,27), "bevel2", B_FOLLOW_BOTTOM);
	m_Bevel_2->MoveTo(2,Bounds().bottom-27);
	m_Box->AddChild(m_Bevel_2);

	// Status bar in the bottom area...
	m_StatusStringView = new BStringView(BRect(2,2,180,20),"status","No file(s) selected.", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	m_StatusStringView->MoveTo(4,Bounds().bottom-25);
	m_StatusStringView->SetAlignment(B_ALIGN_LEFT);
	m_Box->AddChild(m_StatusStringView);

	// Seek TextControl
	m_SeekTextControl = NULL;
	m_SeekMode = false;
	
	BRect r = Bounds();

	r.right-=20;
	r.left+=6;
	r.top+=32;
	r.bottom-=32;

	m_CustomListView = new CustomListView(r, "file_list", this);

	m_pScrollView = new BScrollView("rep_scroll_view", m_CustomListView, B_FOLLOW_ALL, B_WILL_DRAW, false, true);
	m_Box->AddChild(m_pScrollView);
	
	m_CustomListView->SetSelectionMessage(new BMessage(MSG_FILELISTVIEW_SELECTION));

	m_PanelMode = -1;
	SetPanelMode(PM_NORMAL);	
}

////////////////////////////////////////////////////////////////////////
PanelView::~PanelView()
////////////////////////////////////////////////////////////////////////
{
	// To be sure all watcher gone...
	stop_watching(this);

	if (m_UnknownIcon) delete m_UnknownIcon;
	if (m_ParentIcon) delete m_ParentIcon;
}

////////////////////////////////////////////////////////////////////////
void PanelView::AttachedToWindow()
////////////////////////////////////////////////////////////////////////
{
	m_CD_Parent->SetTarget(this);
	m_CD_Root->SetTarget(this);
	m_CD_Home->SetTarget(this);
	m_CD_Desktop->SetTarget(this);
	m_CD_Disks->SetTarget(this);
	
	m_PanelMenu_Find->SetTarget(this);
	m_PanelMenu_ShowIcons->SetTarget(this);
	
	m_CustomListView->SetTarget(this);	// Selection Message!!!
}

////////////////////////////////////////////////////////////////////////
void PanelView::FrameResized(float width, float height)
////////////////////////////////////////////////////////////////////////
{
	BRect r = Bounds();

	m_Box->Invalidate();

	m_Bevel_1->ResizeTo(r.right-4,1);
	m_Bevel_1->Invalidate();

	m_PathStringView->ResizeTo(r.right-44-20,18);
	SetPathStringView();
	m_PathStringView->Invalidate();

	m_CustomListView->ResizeTo(r.right-26,r.bottom-32-32);
	m_CustomListView->Invalidate();
	
	m_Bevel_2->ResizeTo(r.right-4,1);
	
	m_StatusStringView->ResizeTo(r.right-12,18);
	m_StatusStringView->Invalidate();

	if (m_SeekTextControl)
	{
		m_SeekTextControl->ResizeTo(r.right-8,18);
		m_SeekTextControl->Invalidate();
	}
}

/*
void PanelView::SetMousePointer(int n)
{
	// Hourglass...
	char cursordata[] = {
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
*/

////////////////////////////////////////////////////////////////////////
void PanelView::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case MSG_FILE_SEEK:
			SeekModeOn();
			break;
		case MSG_SEEKING:
			SeekFor(m_SeekTextControl->Text());
			break;
		case MSG_FILE_SEEK_END:
			SeekModeOff();
			break;
		case PATH_MSG_CD_PARENT:
			GotoParent();
			break;
		case PATH_MSG_CD_ROOT:
			GotoRoot();
			break;
		case PATH_MSG_CD_HOME:
			GotoHome();
			break;
		case PATH_MSG_CD_DESKTOP:
			GotoDesktop();
			break;
		case PATH_MSG_CD_DISKS:
			if (m_SeekMode)
				SeekModeOff();
			GotoDisks();
			break;
		case MSG_FILELISTVIEW_SELECTION:
			SelectionChanged();
			break;
		case MSG_PANEL_SELECTED:
			m_LastSelectionTime = real_time_clock_usecs();
			Parent()->Looper()->PostMessage(new BMessage(MSG_UPDATEPANEL_SELECTION));	// To update Panels...

			if (m_SeekMode)
				SeekModeOff();
			break;
		case MSG_ENTER:
			Execute(m_CustomListView->GetSelectedEntry(0));
			break;
		case MSG_SPACE:
			Calculate(m_CustomListView->GetSelectedEntry(0));
			break;
		case MSG_VIEW:	// F3
			View();
			break;
		case MSG_EDIT:	// F4
			Edit();
			break;
		case MSG_COPY:	// F5
			Copy();
			break;
		case MSG_MOVE:
			Move();
			break;
		case MSG_RENAME:	// Shift + F6
			Rename();
			break;
		case MSG_MAKEDIR:	// F7
			MakeDir();
			break;
		case MSG_DELETE:	// F8/Del
			Delete();
			break;
		case MSG_QUIT:	// F10
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_RELOAD:	// Reload after MkDir, Copy, Move, Delete...
			{
				BString itemname;
				
				// If there is a given name, let's set the selector to it...
				if (message->FindString("ItemName",&itemname)==B_OK)
					Reload(itemname.String());
				else
					Reload();
//				Rescan();
			}
			break;
		case PANELMENU_MSG_SHOWICONS:
			if (m_PanelMenu_ShowIcons->IsMarked())
			{
				m_PanelMenu_ShowIcons->SetMarked(false);
				m_Setting_ShowIcons = false;
				Reload();
			}
			else
			{
				m_PanelMenu_ShowIcons->SetMarked(true);			
				m_Setting_ShowIcons = true;
				Reload();
			}
			break;
		case B_NODE_MONITOR:
			{
				BEntry entry;
				int32 opcode;
				const char *name;
				entry_ref ref;
				node_ref nref;
				
				if (message->FindInt32("opcode", &opcode) == B_OK)
				{
					switch (opcode)
					{
						case B_ENTRY_CREATED:
							message->FindInt32("device", &ref.device);
							message->FindInt64("directory", &ref.directory);
							message->FindString("name", &name);
							ref.set_name(name);
							entry.SetTo(&ref);
							RescanCreated(&entry);
							break;
						case B_ENTRY_REMOVED:
							message->FindInt32("device", &nref.device);
							message->FindInt64("node", &nref.node);
							RescanRemoved(nref);
							break;
						case B_ENTRY_MOVED:	// or renamed...
							message->FindInt32("device", &nref.device);
							message->FindInt64("node", &nref.node);
							RescanRemoved(nref);
							
							message->FindInt32("device", &ref.device);
							message->FindInt64("to directory", &ref.directory);
							message->FindString("name", &name);
							ref.set_name(name);
							entry.SetTo(&ref);
							RescanCreated(&entry);
							break;
						case B_STAT_CHANGED:
							message->FindInt32("device", &nref.device);
							message->FindInt64("node", &nref.node);
							RescanStat(nref);
							break;
						case B_ATTR_CHANGED:
							message->FindInt32("device", &nref.device);
							message->FindInt64("node", &nref.node);
							RescanAttr(nref);
							break;
						case B_DEVICE_MOUNTED:
							if (m_PanelMode==PM_DISKS)
								Reload();
							break;
						case B_DEVICE_UNMOUNTED:
							if (m_PanelMode==PM_DISKS)
								Reload();
							break;
					}
				}
			}
			break;			
		default:
			BView::MessageReceived(message);
	}	
}

////////////////////////////////////////////////////////////////////////
void PanelView::SaveItemSelection(void)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	int n = m_CustomListView->CountItems();
	
	for (int i=0;i<n;i++)
	{
		item = (CustomListItem *)m_CustomListView->ItemAt(i);
		if (item)
		{
			if (m_CustomListView->IsItemSelected(i))
				item->m_RSelected = true;
			else
				item->m_RSelected = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::LoadItemSelection(void)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	int n = m_CustomListView->CountItems();

	m_CustomListView->SetSelectionMessage(NULL);
	
	for (int i=0;i<n;i++)
	{
		item = (CustomListItem *)m_CustomListView->ItemAt(i);
		if (item)
		{
			if (item->m_RSelected)
				m_CustomListView->Select(i,true);
			else
				m_CustomListView->Deselect(i);
		}
	}

	m_CustomListView->SetSelectionMessage(new BMessage(MSG_FILELISTVIEW_SELECTION));
	Looper()->PostMessage(new BMessage(MSG_FILELISTVIEW_SELECTION), NULL);
}

////////////////////////////////////////////////////////////////////////
void PanelView::Rescan()
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL)
		return;

	if (!DoesEntryExist(m_Path.String()))
	{
		GotoRoot();
		return;
	}

	SaveItemSelection();

	// A listaban meg levo de mar nem letezo fajlokat kitoroljuk a listabol...
	RescanForMissingEntries();
	// Az uj jovevenyeket meg beletesszuk a listaba...
	RescanForNewEntries();

	m_CustomListView->DoSortList();

	LoadItemSelection();
	
	m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();
	SelectionChanged();	
}

////////////////////////////////////////////////////////////////////////
void PanelView::RescanCreated(BEntry *entry)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL)
		return;

	if (!DoesEntryExist(m_Path.String()))
	{
		GotoRoot();
		return;
	}

//	if (entry->InitCheck()!=B_OK)
//		return;
		
	if (!entry->Exists())
		return;

	BEntry parententry;
	BPath path;

	entry->GetParent(&parententry);
	parententry.GetPath(&path);
	if (strcmp(path.Path(),m_Path.String())!=0)
		return;

	SaveItemSelection();

	AddDirectoryEntry(entry, true);		// true = Sorted Insert!
//	m_CustomListView->DoSortList();

	LoadItemSelection();
	
	m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();
//	if (item) m_CurrentTotalSize += item->m_FileSize;
	SelectionChanged();
}

////////////////////////////////////////////////////////////////////////
void PanelView::RescanRemoved(node_ref nref)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL)
		return;

	if (!DoesEntryExist(m_Path.String()))
	{
		GotoRoot();
		return;
	}
	
	CustomListItem *item = m_CustomListView->FindItemByNodeRef(nref);
	if (item)
	{
//		m_CurrentTotalSize -= item->m_FileSize;
		m_CustomListView->RemoveItem(item);
	}

	m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();
	SelectionChanged();
}

////////////////////////////////////////////////////////////////////////
void PanelView::RescanStat(node_ref nref)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL)
		return;

	CustomListItem *item = m_CustomListView->FindItemByNodeRef(nref);
	if (item)
	{
		struct stat statbuf;
		BString fullentryname(m_Path);
		fullentryname << "/" << item->m_FileName;

		BEntry entry(fullentryname.String());
		entry.GetStat(&statbuf);

		item->m_FileSize = statbuf.st_size;
		
		m_CustomListView->InvalidateItem(m_CustomListView->IndexOf(item));

		m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();
		SelectionChanged();	
	}	
}

////////////////////////////////////////////////////////////////////////
void PanelView::RescanAttr(node_ref nref)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL)
		return;

	CustomListItem *item = m_CustomListView->FindItemByNodeRef(nref);
	if (item)
	{
		BString fullentryname(m_Path);
		fullentryname << "/" << item->m_FileName;

		item->GetIcon(fullentryname.String());
		
		m_CustomListView->InvalidateItem(m_CustomListView->IndexOf(item));

		m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();
		SelectionChanged();	
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::RescanForMissingEntries()
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	int n = m_CustomListView->CountItems();
	BEntry entry;
	BString fullname;
	
	for (int i=0;i<n;i++)
	{
		item = (CustomListItem *)m_CustomListView->ItemAt(i);
		if (item)
		{
			fullname = m_Path;
			fullname << "/" << item->m_FileName;
			
			entry.SetTo(fullname.String());
			if (!entry.Exists())
				m_CustomListView->RemoveItem(item);			
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::RescanForNewEntries()
////////////////////////////////////////////////////////////////////////
{
	BDirectory dir(m_Path.String());
	BNode node;
	node_ref noderef;

	if (dir.InitCheck()==B_OK)
	{
		BEntry entry;
		
		if (dir.GetEntry(&entry)==B_OK)
		{
			while (dir.GetNextEntry(&entry)==B_OK)
			{
				node.SetTo(&entry);	
				node.GetNodeRef(&noderef);				
				if (m_CustomListView->FindItemByNodeRef(noderef)==NULL)
					AddDirectoryEntry(&entry);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::EnableMonitoring()
////////////////////////////////////////////////////////////////////////
{
	BEntry entry;
	node_ref nref;
	
	entry.SetTo(m_Path.String());
	if (entry.InitCheck()==B_OK)
	{
		if (entry.Exists())
		{
			entry.GetNodeRef(&nref);
			if (watch_node(&nref, B_WATCH_ALL | B_WATCH_MOUNT , this)==B_OK)
				m_MonitoringPath = m_Path;
			else
				m_MonitoringPath.SetTo("");		// Empty string
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::DisableMonitoring(void)
////////////////////////////////////////////////////////////////////////
{

	BEntry entry;
	node_ref nref;
	
	entry.SetTo(m_Path.String());
	if (entry.InitCheck()==B_OK)
	{
		if (entry.Exists())
		{
			entry.GetNodeRef(&nref);
			if (watch_node(&nref, B_STOP_WATCHING , this)==B_OK)
				m_MonitoringPath.SetTo("");
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::LoadResources(void)
////////////////////////////////////////////////////////////////////////
{
	entry_ref	ref;
	app_info 	info;

	m_ParentIcon = NULL;
	m_UnknownIcon = NULL;

	if (be_app->GetAppInfo(&info)==B_OK)
	{
		BFile file(&info.ref, B_READ_ONLY);
		
		if (file.InitCheck()==B_OK)
		{
			BResources rsrcs;
			size_t len = 0;
			
			if (rsrcs.SetTo(&file)==B_OK)
			{
				const void *data;
				data = rsrcs.LoadResource('MICN',1,&len);
				if (data)
				{
					m_ParentIcon = new unsigned char[len];
					memcpy(m_ParentIcon,data,len);
				}					

				data = rsrcs.LoadResource('MICN',2,&len);
				if (data)
				{
					m_UnknownIcon = new unsigned char[len];
					memcpy(m_UnknownIcon,data,len);
				}
			}
		}
	}	
}

////////////////////////////////////////////////////////////////////////
void PanelView::ClearFileList()
////////////////////////////////////////////////////////////////////////
{
	m_CustomListView->MakeEmpty();
}

////////////////////////////////////////////////////////////////////////
void PanelView::ReadDirectory(const char *itemname)
////////////////////////////////////////////////////////////////////////
{
	BDirectory *dir;
	CustomListItem *item;

	stop_watching(this);
//	SetMousePointer(CR_HOURGLASS);
	MAINWINDOW->SetMousePointer(GenesisWindow::CR_HOURGLASS);
	
	// If we are not in the root directory, simply start the dir with a '..' entry...
	if (strcmp(m_Path.String(),"/")!=0)
	{
		item = new CustomListItem("..", m_Path.String(), FT_PARENT, 0);
		m_CustomListView->AddItem(item);
		if (m_Setting_ShowIcons)
		{
			item->AddIcon(m_ParentIcon);
			item->SetHeight(15.0f);
		}
	}
	
	dir = new BDirectory(m_Path.String());
	if (dir)
	{
		BEntry entry;
		
		if (dir->GetEntry(&entry)==B_OK)
		{	
			while (dir->GetNextEntry(&entry)==B_OK)			
			{
				AddDirectoryEntry(&entry);
			}
		}
	
		delete dir;
	}
	
	m_CustomListView->DoSortList();
	
	// Always select the first item in the list or the child where we came from...
	if (itemname)
	{
		CustomListItem *item;
		int n = m_CustomListView->CountItems();
		
		for (int i=0;i<n;i++)
		{
			item = (CustomListItem *)m_CustomListView->ItemAt(i);

			if (strcasecmp(itemname,item->m_FileName.String())==0)
			{
				m_CustomListView->Select(i,false);
				m_CustomListView->ScrollToSelection();
				break;
			}			
		}
		
		// When the given file disappeared, we have to select the first entry...
		if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==0)
			m_CustomListView->Select(0,false);	
	}
	else
		m_CustomListView->Select(0,false);

	m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();

	// Probably we have to update the path of the command line...
	Parent()->Looper()->PostMessage(new BMessage(MSG_UPDATECOMMANDLINE_PATH));	// To update command line...

	EnableMonitoring();
	
//	SetMousePointer(CR_DEFAULT);
	MAINWINDOW->SetMousePointer(GenesisWindow::CR_DEFAULT);
}

////////////////////////////////////////////////////////////////////////
CustomListItem *PanelView::AddDirectoryEntry(BEntry *entry,  bool sorted)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	char name[B_FILE_NAME_LENGTH];

	entry->GetName(name);

	if (entry->IsDirectory())
	{
		item = new CustomListItem(name,m_Path.String(),FT_DIRECTORY, 0, this);
		if (sorted)
			m_CustomListView->AddSortedItem(item);
		else	
			m_CustomListView->AddItem(item);
		if (m_Setting_ShowIcons)
		{
			if (!item->GetIcon(entry))
				item->AddIcon(m_UnknownIcon);
			item->SetHeight(15.0f);
		}
	}
	else if (entry->IsSymLink())
	{
		BEntry symlinkentry;
		entry_ref ref;		
		struct stat statbuf;
	
		entry->GetRef(&ref);
		if (symlinkentry.SetTo(&ref, true)==B_OK)
		{
			if (symlinkentry.IsDirectory())
			{
				item = new CustomListItem(name,m_Path.String(),FT_SYMLINKDIR, 0, this);
				if (sorted)
					m_CustomListView->AddSortedItem(item);
				else	
					m_CustomListView->AddItem(item);
				if (m_Setting_ShowIcons)
				{
					if (!item->GetIcon(&symlinkentry))
						item->AddIcon(m_UnknownIcon);
					item->SetHeight(15.0f);
				}
			}
			else
			{
				symlinkentry.GetStat(&statbuf);
				item = new CustomListItem(name,m_Path.String(),FT_SYMLINKFILE,statbuf.st_size, this);
				if (sorted)
					m_CustomListView->AddSortedItem(item);
				else	
					m_CustomListView->AddItem(item);
				if (m_Setting_ShowIcons)
				{
					if (!item->GetIcon(&symlinkentry))
						item->AddIcon(m_UnknownIcon);
					item->SetHeight(15.0f);
				}
			}
		}
		else
		{
			// Broken link...
			item = new CustomListItem(name, m_Path.String(), FT_SYMLINKBROKEN, 0 , this);
			if (sorted)
				m_CustomListView->AddSortedItem(item);
			else	
				m_CustomListView->AddItem(item);
			if (m_Setting_ShowIcons)
			{					
				if (!item->GetIcon(entry))
					item->AddIcon(m_UnknownIcon);
				item->SetHeight(15.0f);
			}
		}
	}
	else
	{
		struct stat statbuf;
		entry->GetStat(&statbuf);
		
		item = new CustomListItem(name,m_Path.String(),FT_FILE,statbuf.st_size, this);
		if (sorted)
			m_CustomListView->AddSortedItem(item);
		else	
			m_CustomListView->AddItem(item);
		if (m_Setting_ShowIcons)
		{
			if (!item->GetIcon(entry))
				item->AddIcon(m_UnknownIcon);
			item->SetHeight(15.0f);
		}
	}
	
	return item;
}

////////////////////////////////////////////////////////////////////////
void PanelView::ReadDisks(void)
////////////////////////////////////////////////////////////////////////
{
	char drivename[256];
	char drivepath[256];

//	SetMousePointer(CR_HOURGLASS);
	MAINWINDOW->SetMousePointer(GenesisWindow::CR_HOURGLASS);

	CustomListItem *item;

	item = new CustomListItem("..",m_Path.String(),FT_DISKBACK, 0);
	item->AddIcon(m_ParentIcon);
	m_CustomListView->AddItem(item);
	item->SetHeight(15.0f);

	// Collect available volumes...
	BVolumeRoster *vr = new BVolumeRoster();
	if (vr)
	{
		BVolume v;
	
		while (vr->GetNextVolume(&v)==B_NO_ERROR)
		{
			if (v.GetName(drivename)==B_NO_ERROR)
			{
				if (strlen(drivename)>0)
				{
					BDirectory dir;
					BEntry entry;
					BPath path;
					v.GetRootDirectory(&dir);
					dir.GetEntry(&entry);
					entry.GetPath(&path);
					sprintf(drivepath,"%s",path.Path());
					item = new CustomListItem(drivename,drivepath,FT_DISKITEM,v.FreeBytes(),v.Capacity(),v.Device());
					m_CustomListView->AddItem(item);
					if (m_Setting_ShowIcons)
					{
						if (!item->GetIcon(&v))
							item->AddIcon(m_UnknownIcon);
						item->SetHeight(15.0f);
					}
				}
			}
		}

		delete vr;
	}

	m_CustomListView->DoSortList();

	m_CustomListView->Select(0,false);
//	SetMousePointer(CR_DEFAULT);
	MAINWINDOW->SetMousePointer(GenesisWindow::CR_DEFAULT);
}

////////////////////////////////////////////////////////////////////////
void PanelView::SelectAll(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode==PM_DISKS) return;

	int items = m_CustomListView->CountItems();

	m_CustomListView->Select(0,items-1,false);
}

////////////////////////////////////////////////////////////////////////
void PanelView::DeselectAll(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode==PM_DISKS) return;
	m_CustomListView->DeselectAll();
}

////////////////////////////////////////////////////////////////////////
void PanelView::InvertSelection(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode==PM_DISKS) return;

	int n = m_CustomListView->CountItems();

	// We have to clear the selection message to avoid message queue from overrun...
	m_CustomListView->SetSelectionMessage(NULL);

	for (int i=0;i<n;i++)
	{
		switch(m_CustomListView->IsItemSelected(i))
		{
			case true:
				m_CustomListView->Deselect(i);
				break;
			case false:
				m_CustomListView->Select(i,true);
				break;
		}
	}

	// Let's set again the selection message,
	m_CustomListView->SetSelectionMessage(new BMessage(MSG_FILELISTVIEW_SELECTION));
	// and generate a selection changed message...
	Looper()->PostMessage(new BMessage(MSG_FILELISTVIEW_SELECTION), NULL);
}

////////////////////////////////////////////////////////////////////////
void PanelView::AddToSelection(ADDTYPES type)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode==PM_DISKS) return;

	CustomListItem *item;
	int n = m_CustomListView->CountItems();

	// We have to clear the selection message to avoid message queue from overrun...
	m_CustomListView->SetSelectionMessage(NULL);

	for (int i=0;i<n;i++)
	{
		item = (CustomListItem *)m_CustomListView->ItemAt(i);
		if (item) switch (type)
		{
			case ADD_FOLDERS:
				if (item->m_Type == FT_DIRECTORY)
					m_CustomListView->Select(i,true);
				break;
			case ADD_FILES:
				if (item->m_Type == FT_FILE)
					m_CustomListView->Select(i,true);
				break;
			case ADD_SYMLINKS:
				if (item->m_Type == FT_SYMLINKFILE ||
					item->m_Type == FT_SYMLINKDIR ||
					item->m_Type == FT_SYMLINKBROKEN )
					m_CustomListView->Select(i,true);
				break;		
		}
	}

	// Let's set again the selection message,
	m_CustomListView->SetSelectionMessage(new BMessage(MSG_FILELISTVIEW_SELECTION));
	// and generate a selection changed message...
	Looper()->PostMessage(new BMessage(MSG_FILELISTVIEW_SELECTION), NULL);
}


////////////////////////////////////////////////////////////////////////
bool PanelView::DoesEntryExist(const char *filename)
////////////////////////////////////////////////////////////////////////
{
	BEntry entry;
	
	entry.SetTo(filename);
	if (entry.InitCheck()==B_OK)
	{
		if (entry.Exists())
			return true;
		else
			return false;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
void PanelView::ChangePath(const char *p)
////////////////////////////////////////////////////////////////////////
{
	if (DoesEntryExist(p))
	{
		m_Path.SetTo(p);

		SetPathStringView();
		ClearFileList();
		ReadDirectory();
	}
	else
		GotoRoot();
}

////////////////////////////////////////////////////////////////////////
void PanelView::EnterDirectory(const char *p)
////////////////////////////////////////////////////////////////////////
{
	char buf[256];		// TODO: mi a leghosszabb? Ki kellene keresni...
	int len = m_Path.Length();

	// Ha mar van elotte / jel...
	if (m_Path.ByteAt(len-1)=='/')
		sprintf(buf,"%s%s",m_Path.String(),p);
	else
		sprintf(buf,"%s/%s",m_Path.String(),p);

	if (DoesEntryExist(p))
	{
		m_Path.SetTo(buf);
	
		SetPathStringView();
		ClearFileList();
		ReadDirectory();
	}
	else
		GotoRoot();
}

////////////////////////////////////////////////////////////////////////
void PanelView::GotoParent(void)
////////////////////////////////////////////////////////////////////////
{
	// To set the cursor to the child entry where we came from...
	BString oldpath;
	SetPanelMode(PM_NORMAL);
	oldpath.SetTo(m_Path.String());
	oldpath.Remove(0,oldpath.FindLast('/')+1);
	
	BPath path((const char *)m_Path.String());
	
	if (path.GetParent(&path)==B_OK)
		m_Path.SetTo(path.Path());

	SetPathStringView();
	ClearFileList();
	ReadDirectory(oldpath.String());
}

////////////////////////////////////////////////////////////////////////
void PanelView::GotoRoot(void)
////////////////////////////////////////////////////////////////////////
{
	SetPanelMode(PM_NORMAL);
	m_Path.SetTo("/");
	SetPathStringView();
	ClearFileList();
	ReadDirectory();
}

////////////////////////////////////////////////////////////////////////
void PanelView::GotoHome(void)
////////////////////////////////////////////////////////////////////////
{
	BPath dirPath;
	find_directory(B_USER_DIRECTORY, &dirPath, true);

	SetPanelMode(PM_NORMAL);
	m_Path.SetTo(dirPath.Path());
	SetPathStringView();
	ClearFileList();
	ReadDirectory();
}

////////////////////////////////////////////////////////////////////////
void PanelView::GotoDesktop(void)
////////////////////////////////////////////////////////////////////////
{
	BPath dirPath;
	find_directory(B_DESKTOP_DIRECTORY, &dirPath, true);

	SetPanelMode(PM_NORMAL);
	m_Path.SetTo(dirPath.Path());
	SetPathStringView();
	ClearFileList();
	ReadDirectory();
}

////////////////////////////////////////////////////////////////////////
void PanelView::GotoDisks(void)
////////////////////////////////////////////////////////////////////////
{
	SetPanelMode(PM_DISKS);
	m_PathStringView->SetText("Mounted disks:");
	ClearFileList();
	ReadDisks();
}

////////////////////////////////////////////////////////////////////////
void PanelView::Reload(void)
////////////////////////////////////////////////////////////////////////
{
	switch (m_PanelMode)
	{
		case PM_NORMAL:
			ClearFileList();
			ReadDirectory();
			break;
		case PM_DISKS:
			ClearFileList();
			ReadDisks();
			break;
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::Reload(const char *itemname)
////////////////////////////////////////////////////////////////////////
{
	switch (m_PanelMode)
	{
		case PM_NORMAL:
			ClearFileList();
			ReadDirectory(itemname);
			break;
		case PM_DISKS:
			ClearFileList();
			ReadDisks();
			break;
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::Reload(int index)
////////////////////////////////////////////////////////////////////////
{
	switch (m_PanelMode)
	{
		case PM_NORMAL:
			ClearFileList();
			ReadDirectory();
			if (index > (m_CustomListView->IndexOf(m_CustomListView->LastItem())))
				index = m_CustomListView->IndexOf(m_CustomListView->LastItem());
			
			m_CustomListView->Select(index,false);	// false -> remove previously selected item(s)...
			break;
		case PM_DISKS:
			ClearFileList();
			ReadDisks();
			m_CustomListView->Select(index,false);	// false -> remove previously selected item(s)...
			break;
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::DeleteDirectory(const char *dirname)
////////////////////////////////////////////////////////////////////////
{
	BDirectory *dir;
	key_info keyinfo;
	
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
		
		if (dir->GetEntry(&entry)==B_OK)
		{	
			while (dir->GetNextEntry(&entry)==B_OK)			
			{
				get_key_info(&keyinfo);
				if (keyinfo.key_states[0] & 0x40)	// ESC
				{
					beep();
					delete dir;
					return;
				}

				BPath path;
				entry.GetPath(&path);
				
				if (entry.IsDirectory())
					DeleteDirectory(path.Path());

				entry.Remove();
			}
		}
	
		delete dir;
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::UniversalDelete(const char *filename)
////////////////////////////////////////////////////////////////////////
{
	BEntry entry(filename);

	key_info keyinfo;
	get_key_info(&keyinfo);
	if (keyinfo.key_states[0] & 0x40)
	{
		beep();
		return;	// ESC
	}

	// Don't delete the parent directory!!!!!!
	if (strlen(filename)>=3)
	{
		int len = strlen(filename);
		if (filename[len-1]=='.' && filename[len-2]=='.' && filename[len-3]=='/') return;
	}

	if (entry.InitCheck()==B_OK)
	{
		if (entry.Exists())
		{
			if (entry.IsDirectory())
				DeleteDirectory(filename);
		
			entry.Remove();
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::WriteFileSize(BString *str, uint64 FileSize)
////////////////////////////////////////////////////////////////////////
{
	char buf[64];
	BString tempstring;
	tempstring.SetTo("");

	if (FileSize>=(1024*1024*1024))
	{
		tempstring << FileSize/(1024.0f*1024.0f*1024.0f);
		sprintf(buf,"%s GB",tempstring.String());
	}
	else if (FileSize>=(1024*1024))
	{
		tempstring << FileSize/(1024.0f*1024.0f);
		sprintf(buf,"%s MB",tempstring.String());	
	}
	else if (FileSize>=1024)
	{
		tempstring << FileSize/1024.0f;
		sprintf(buf,"%s KB",tempstring.String());
	}
	else
	{
		tempstring << FileSize;
		sprintf(buf,"%s byte%s",tempstring.String(),FileSize<=1?"":"s");
	}
		
	str->SetTo(buf);
}

////////////////////////////////////////////////////////////////////////
void PanelView::SelectionChanged(void)
////////////////////////////////////////////////////////////////////////
{
	char buf[256];
	BString totalsizestr;

	switch (m_PanelMode)
	{
		case PM_NORMAL:
			{
				int n = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);
				int total = m_CustomListView->CountEntries(CT_WITHOUTPARENT);
	
				CustomListItem *item;

				WriteFileSize(&totalsizestr,m_CurrentTotalSize);

				if (n==0)
				{
					sprintf(buf,"No files selected - Total: %d file%s in %s",total,total<=1?"":"s",totalsizestr.String());
				}
				else if (n==1)
				{
					item = m_CustomListView->GetSelectedEntry(0);
		
					BString selstr,totalstr;
				
					if (item)
					{
						WriteFileSize(&selstr,item->m_FileSize);
						WriteFileSize(&totalstr,total);
						sprintf(buf,"%d file%s selected in %s - Total: %d file%s in %s",n,n<=1?"":"s",selstr.String(),total,total<=1?"":"s",totalsizestr.String());
					}
					else 
						sprintf(buf,"Internal error...");
				}
				else
				{	
					BString seltotalsizestr;
	
					WriteFileSize(&seltotalsizestr,m_CustomListView->GetSelectedTotalSize());
		
					sprintf(buf,"%d file%s selected in %s - Total: %d file%s in %s", n, n<=1?"":"s", seltotalsizestr.String(), total, total<=1?"":"s", totalsizestr.String());
				}
	
				m_StatusStringView->SetText(buf);
			}
			break;
		case PM_DISKS:
			{
				CustomListItem *item;
				item = m_CustomListView->GetSelectedEntry(0);
				if (item)
				{
					if (item->m_Type == FT_DISKBACK)
					{
						sprintf(buf,"Back to %s",m_Path.String());
					}
					else
					{
						if (item->m_Capacity>=(1024*1024*1024))
							sprintf(buf,"%s - Capacity: %.02f GB",item->m_DiskPath.String(),(float)item->m_Capacity/(1024.0*1024.0*1024.0));
						else 
							sprintf(buf,"%s - Capacity: %.02f MB",item->m_DiskPath.String(),(float)item->m_Capacity/(1024.0*1024.0));
					}			
				}
				else
					sprintf(buf,"No disks selected.");
				
				m_StatusStringView->SetText(buf);
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////
// When the user pressed the Enter or double-clicked...
void PanelView::Execute(CustomListItem *item)
////////////////////////////////////////////////////////////////////////
{
	BString execute;

	switch (item->m_Type)
	{
		case FT_DISKITEM:
			SetPanelMode(PM_NORMAL);
			ChangePath(item->m_DiskPath.String());			
			break;
		case FT_DISKBACK:
			SetPanelMode(PM_NORMAL);
			ChangePath(m_Path.String());
			break;
		case FT_PARENT:
			GotoParent();
			break;
		case FT_DIRECTORY:
			EnterDirectory(item->m_FileName.String());
			break;
		case FT_SYMLINKDIR:
		case FT_SYMLINKFILE:	// TODO: ha mar ugyis tudjuk elore....
			{
				BEntry symlinkentry;
				entry_ref ref;

				execute.SetTo(m_Path.String());
				execute+="/";
				execute+=item->m_FileName;

				BEntry entry(execute.String());
				entry.GetRef(&ref);
				symlinkentry.SetTo(&ref, true);
				if (symlinkentry.IsDirectory())		// Is it a directory?
				{
					BPath symlinkpath;
					symlinkentry.GetPath(&symlinkpath);
					ChangePath(symlinkpath.Path());
				}
				else	// No, it is a standard file....
				{
					// Launch the selected file or document...
					if (entry.GetRef(&ref) == B_OK)
						be_roster->Launch(&ref);
				}
			}
			break;
		case FT_FILE:
			{
				// Launch the selected file or document...
				entry_ref ref;

				execute.SetTo(m_Path.String());
				execute+="/";
				execute+=item->m_FileName;

				BEntry entry(execute.String());
				if (entry.GetRef(&ref) == B_OK)
					be_roster->Launch(&ref);
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::Calculate(CustomListItem *item)
////////////////////////////////////////////////////////////////////////
{
//	SetMousePointer(CR_HOURGLASS);
	MAINWINDOW->SetMousePointer(GenesisWindow::CR_HOURGLASS);

	if (item->m_Type==FT_DIRECTORY)
	{
		BString file;

		file.SetTo(m_Path.String());
		file+="/";
		file+=item->m_FileName;

		item->m_FileSize = GetDirectorySize(file.String());
		m_CustomListView->InvalidateItem(m_CustomListView->IndexOf(item));
	}
	
	m_CurrentTotalSize = m_CustomListView->GetCurrentTotalSize();
	SelectionChanged();		// Because the total directory size may change...
	
//	SetMousePointer(CR_DEFAULT);
	MAINWINDOW->SetMousePointer(GenesisWindow::CR_DEFAULT);
}

////////////////////////////////////////////////////////////////////////
void PanelView::SetPathStringView(void)
////////////////////////////////////////////////////////////////////////
{
	BString text, path;

	text = path = m_Path;
	
	int sw = (int)m_PathStringView->StringWidth(text.String());
	int w = (int)m_PathStringView->Bounds().Width();

	while (sw>w)
	{
		text.SetTo("...");
		text += path.Remove(0,1);
		
		sw = (int)m_PathStringView->StringWidth(text.String());
	}

	m_PathStringView->SetText(text.String());
}

////////////////////////////////////////////////////////////////////////
void PanelView::GetInfo(void)
////////////////////////////////////////////////////////////////////////
{
	GenesisGetInfoWindow *infowindow;
	GenesisGetDiskInfoWindow *diskinfowindow;

	if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==1)
	{
		BString file;

		CustomListItem *item = m_CustomListView->GetSelectedEntry(0);
		if (item)
		{
			switch (item->m_Type)
			{
				case FT_FILE:
				case FT_SYMLINKFILE:
				case FT_SYMLINKDIR:
				case FT_DIRECTORY:
					file.SetTo(m_Path.String());
					file+="/";
					file+=item->m_FileName;
	
					infowindow = new GenesisGetInfoWindow(file.String(), Window());
					infowindow->Show();
					break;
				case FT_DISKITEM:
					diskinfowindow = new GenesisGetDiskInfoWindow(item, Window());
					diskinfowindow->Show();
					break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::SetPanelMode(int mode)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode == mode)
		return;

	m_PanelMode = mode;
	switch (m_PanelMode)
	{
		case PM_NORMAL:
			m_CustomListView->SetListType(B_MULTIPLE_SELECTION_LIST);
			break;
		case PM_DISKS:
			m_CustomListView->SetListType(B_SINGLE_SELECTION_LIST);
			break;
	}
}

////////////////////////////////////////////////////////////////////////
uint64 PanelView::GetDirectorySize(const char *path)
////////////////////////////////////////////////////////////////////////
{
	uint64 size = 0;
	BDirectory *dir;
		
	dir = new BDirectory(path);
	if (dir)
	{
		BEntry entry;
		
		if (dir->GetEntry(&entry)==B_OK)
		{	
			while (dir->GetNextEntry(&entry)==B_OK)			
			{
				BPath path;
				entry.GetPath(&path);
				
				if (entry.IsDirectory())
					size += GetDirectorySize(path.Path());
				else
				{
					struct stat statbuf;
					entry.GetStat(&statbuf);
					
					size += statbuf.st_size;
				}	
			}
		}
	
		delete dir;
	}

	return size;
}

////////////////////////////////////////////////////////////////////////
void PanelView::CreateLinkOnDesktop(void)
////////////////////////////////////////////////////////////////////////
{
	BString text;
	CustomListItem *item;
	
	if (m_PanelMode != PM_NORMAL) return;

	int n = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);

	// Ha a Parent-en all...
	if (n==0)
		return;

	// Let's construct the warning message...
	if (n==1)
	{
		item = m_CustomListView->GetSelectedEntry(0);	
		if (item->m_Type==FT_PARENT) // Azon ritka esetek egyike amivel meg lehetett szivatni a programot...
			item = m_CustomListView->GetSelectedEntry(1);

		text.SetTo("Do you want to create a symbolic link on the Desktop for '");
		text+=item->m_FileName;
		text+="'?";
	}
	else
	{
		text.SetTo("Do you want to create symbolic links on the Desktop for ");
		text << n;
		text+=" files?";
	}

	// Let's display the warning message...
	BAlert *WarningAlert = new BAlert("Confirm",text.String(),"Cancel","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
	WarningAlert->SetShortcut(0, B_ESCAPE);
	if (WarningAlert->Go()==1)
	{
		BString name;
		BString linkpath;
		BSymLink dstlink;
		BPath DesktopPath;
		find_directory(B_DESKTOP_DIRECTORY, &DesktopPath, true);
		
		BDirectory dstdir(DesktopPath.Path());
		
		item = m_CustomListView->GetSelectedEntry(0);
		
		while (item)
		{
			if (item->m_Type==FT_PARENT)
			{
				item = m_CustomListView->GetSelectedEntry(0);
				continue;
			}
		
			name.SetTo(item->m_FileName);
			name << " link";
		
			linkpath.SetTo(item->m_FilePath.String());
			linkpath << "/" << item->m_FileName;
		
			dstdir.CreateSymLink(name.String(), linkpath.String(), &dstlink);
			
			m_CustomListView->Deselect(m_CustomListView->IndexOf(item));
			m_CustomListView->InvalidateItem(m_CustomListView->IndexOf(item));

			item = m_CustomListView->GetSelectedEntry(0);
		}
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::View()
////////////////////////////////////////////////////////////////////////
{
	GenesisViewWindow *viewwindow;

	if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==1)
	{
		BString file;

		CustomListItem *item = m_CustomListView->GetSelectedEntry(0);
		if (item)
		{
			switch (item->m_Type)
			{
				case FT_FILE:
				case FT_SYMLINKFILE:
					file.SetTo(m_Path.String());
					file+="/";
					file+=item->m_FileName;

					viewwindow = new GenesisViewWindow(file.String(), Window());
					viewwindow->Show();
					break;
			}
		}
	}	
}

////////////////////////////////////////////////////////////////////////
void PanelView::Edit()
////////////////////////////////////////////////////////////////////////
{
	if (m_CustomListView->CountSelectedEntries(CT_WITHPARENT)==1)
	{
		BString file;

		CustomListItem *item = m_CustomListView->GetSelectedEntry(0);
		if (item)
		{
			switch (item->m_Type)
			{
				case FT_FILE:
				case FT_SYMLINKFILE:
					file.SetTo(m_Path.String());
					file+="/";
					file+=item->m_FileName;
					
					BString tempstring;
					tempstring.SetTo("StyledEdit \"");
					tempstring << file.String();
					tempstring << "\" &";
					system(tempstring.String());
					
					break;
			}
		}
	}	
}

////////////////////////////////////////////////////////////////////////
void PanelView::MakeDir(void)
////////////////////////////////////////////////////////////////////////
{
	GenesisMakeDirWindow *makedirwindow;

	makedirwindow = new GenesisMakeDirWindow(m_Path.String(), Looper(), Window());
	makedirwindow->Show();
}

////////////////////////////////////////////////////////////////////////
void PanelView::Delete(void)
////////////////////////////////////////////////////////////////////////
{
	BString text;
	CustomListItem *item;
	GenesisDeleteWindow *deletewindow;
	
	if (m_PanelMode != PM_NORMAL) return;

	int n = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);

	if (n==0)
		return;

	// Let's construct the warning message...
	if (n==1)
	{
		item = m_CustomListView->GetSelectedEntry(0);	
		if (item->m_Type==FT_PARENT) // Azon ritka esetek egyike amivel meg lehetett szivatni a programot...
			item = m_CustomListView->GetSelectedEntry(1);

		text.SetTo("Do you realy want to delete '");
		text+=item->m_FileName;
		text+="'?";
	}
	else
	{
		text.SetTo("Do you realy want to delete ");
		text << n;
		text+=" files?";
	}

	// Let's display the warning message...
	BAlert *WarningAlert = new BAlert("Confirm",text.String(),"Cancel","Yes",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
	WarningAlert->SetShortcut(0, B_ESCAPE);
	if (WarningAlert->Go()==1)
	{
//		int x = m_CustomListView->IndexOf(m_CustomListView->GetSelectedEntry(0));

		deletewindow = new GenesisDeleteWindow(m_CustomListView, Looper(), Window());
		deletewindow->Go();
//		Reload(x);
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::Copy(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL) return;

	int n = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);
	if (n>0)
	{
		GenesisCopyWindow *copywindow;
		BString destpath;

		PanelView *destpanel = ((GenesisWindow *)Window())->GetInactivePanel();
		if (destpanel)
		{
			destpath.SetTo(destpanel->m_Path.String());
		}
		else
			destpath.SetTo("/");

		copywindow = new GenesisCopyWindow(m_CustomListView, destpanel, destpath.String(), Looper(), Window());
		copywindow->Go();
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::Rename(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL) return;

	if (m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT)>0)
	{
		GenesisRenameWindow *renamewindow;
	
		renamewindow = new GenesisRenameWindow(m_CustomListView, Window());
		renamewindow->Go();
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::Move(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL) return;

	int n = m_CustomListView->CountSelectedEntries(CT_WITHOUTPARENT);
	if (n>0)
	{
		GenesisMoveWindow *movewindow;
		BString destpath;

		PanelView *destpanel = ((GenesisWindow *)Window())->GetInactivePanel();
		if (destpanel)
		{
			destpath.SetTo(destpanel->m_Path.String());
		}
		else
			destpath.SetTo("/");

		movewindow = new GenesisMoveWindow(m_CustomListView, destpanel, destpath.String(), Looper(), Window());
		movewindow->Go();
	}
}

////////////////////////////////////////////////////////////////////////
void PanelView::SeekModeOn(void)
////////////////////////////////////////////////////////////////////////
{
	if (m_PanelMode != PM_NORMAL) return;
	if (m_SeekMode == true) return;		// Avoid multiple creations...

	m_SeekMode = true;
	m_StatusStringView->Hide();

	m_SeekTextControl = new SeekControl(BRect(2,2,180,20), "seek", new BMessage(MSG_FILE_SEEK_END));
	m_SeekTextControl->SetModificationMessage(new BMessage(MSG_SEEKING));
	m_SeekTextControl->SetTarget(this);
	m_Box->AddChild(m_SeekTextControl);
	m_SeekTextControl->MoveTo(4,Bounds().bottom-23);
	m_SeekTextControl->ResizeTo(Bounds().right-8,18);
	m_SeekTextControl->MakeFocus(true);
}

////////////////////////////////////////////////////////////////////////
void PanelView::SeekModeOff(void)
////////////////////////////////////////////////////////////////////////
{
	m_SeekMode = false;

	if (m_SeekTextControl)
	{
		m_SeekTextControl->RemoveSelf();
		m_SeekTextControl = NULL;
	}
	m_StatusStringView->Show();
	m_CustomListView->MakeFocus(true);
}

////////////////////////////////////////////////////////////////////////
void PanelView::SeekFor(const char *text)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	int n = m_CustomListView->CountItems();

	if (m_PanelMode != PM_NORMAL) return;
	
	m_CustomListView->DeselectAll();
	
	for (int i=0;i<n;i++)
	{
		item = (CustomListItem *)m_CustomListView->ItemAt(i);
		if (item)
		{
			if (strncmp(item->m_FileName.String(),text,strlen(text))==0)
			{
				m_CustomListView->Select(i,false);
				m_CustomListView->ScrollToSelection();
				break;
			}
		}
	}
}

