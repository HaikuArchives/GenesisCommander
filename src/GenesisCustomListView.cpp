/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#include "GenesisCustomListView.h"
#include "GenesisCustomListItem.h"
#include "GenesisWindow.h"
#include "GenesisPanelView.h"
#include <stdio.h>
#include <string.h>
#include <View.h>
#include <Font.h>
#include <Beep.h>
#include <InterfaceDefs.h>
#include <Application.h>
#include <Alert.h>

////////////////////////////////////////////////////////////////////////
CustomListView::CustomListView(BRect rect, char *name, void *pv)
	   	   : BListView( rect, name , B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE)
////////////////////////////////////////////////////////////////////////
{
	DirectoryColor.red = 0;
	DirectoryColor.green = 0;
	DirectoryColor.blue = 128;
	DirectoryColor.alpha = 255;
	
	SymLinkColor.red = 0;
	SymLinkColor.green = 64;
	SymLinkColor.blue = 64;
	SymLinkColor.alpha = 255;
	
	get_click_speed( &m_ClickSpeed ); 
	
	m_LastClicked = system_time();
	m_LastClickedItem = -1;
	
	m_PV = pv;
}

////////////////////////////////////////////////////////////////////////
CustomListView::~CustomListView()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
bool CustomListView::GetBoolSetting(SETTING n)
////////////////////////////////////////////////////////////////////////
{
	switch (n)
	{
		case SETTING_SHOWICON:
			return ((PanelView *)m_PV)->m_Setting_ShowIcons;
		default:
			return false;
	}
}

////////////////////////////////////////////////////////////////////////
int CustomListView::CompareFunc( const void *first, const void *second)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item1 = (*(static_cast<CustomListItem * const*>(first )));
	CustomListItem *item2 = (*(static_cast<CustomListItem * const*>(second)));

	// The '..' item is always the first one...	
	if (item1->m_Type==FT_PARENT) return -1;
	else if (item2->m_Type==FT_PARENT) return 1;
	
	// Directories first...
	if ((item1->m_Type==FT_DIRECTORY || item1->m_Type==FT_SYMLINKDIR) && (item2->m_Type!=FT_DIRECTORY && item2->m_Type!=FT_SYMLINKDIR)) return -1;
	else if ((item2->m_Type==FT_DIRECTORY || item2->m_Type==FT_SYMLINKDIR) && (item1->m_Type!=FT_DIRECTORY && item1->m_Type!=FT_SYMLINKDIR)) return 1;
		
	return strcasecmp(item1->m_FileName.String(),item2->m_FileName.String());
}

////////////////////////////////////////////////////////////////////////
int CustomListView::CompareFunc( CustomListItem *first, CustomListItem *second)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item1 = first;
	CustomListItem *item2 = second;

	// The '..' item is always the first one...	
	if (item1->m_Type==FT_PARENT) return -1;
	else if (item2->m_Type==FT_PARENT) return 1;
	
	// Directories first...
	if ((item1->m_Type==FT_DIRECTORY || item1->m_Type==FT_SYMLINKDIR) && (item2->m_Type!=FT_DIRECTORY && item2->m_Type!=FT_SYMLINKDIR)) return -1;
	else if ((item2->m_Type==FT_DIRECTORY || item2->m_Type==FT_SYMLINKDIR) && (item1->m_Type!=FT_DIRECTORY && item1->m_Type!=FT_SYMLINKDIR)) return 1;
		
	return strcasecmp(item1->m_FileName.String(),item2->m_FileName.String());
}

////////////////////////////////////////////////////////////////////////
void CustomListView::DoSortList()
////////////////////////////////////////////////////////////////////////
{
	SortItems(CompareFunc);
}

////////////////////////////////////////////////////////////////////////
void CustomListView::AttachedToWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void CustomListView::MakeFocus(bool focusState)
////////////////////////////////////////////////////////////////////////
{
	if (focusState==true)
		((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_PANEL_SELECTED), NULL);

	BListView::MakeFocus(focusState);
}

////////////////////////////////////////////////////////////////////////
void CustomListView::KeyDown(const char *bytes, int32 numBytes)
////////////////////////////////////////////////////////////////////////
{
	int process = true;
	key_info keyinfo;

	for (int i=0;i<numBytes;i++)
	{
		switch (bytes[i])
		{
			case B_FUNCTION_KEY:
				{
					if (get_key_info(&keyinfo)==B_OK)
					{
//						char buf[256];
//						sprintf(buf,"info:%x %x %x %x %x %x",keyinfo.key_states[0],keyinfo.key_states[1],keyinfo.key_states[2],keyinfo.key_states[3],keyinfo.key_states[4],keyinfo.key_states[5]);
//						BAlert *myAlert = new BAlert("DebugInfo",buf,"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
//						myAlert->Go();

						if (modifiers() & B_SHIFT_KEY)
						{
							// Shift + F6 - Rename
							if (keyinfo.key_states[0] & 0x1)
							{
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_RENAME), NULL);
							}
						}
						else
						{
							// F1 - CD menu popup
							if (keyinfo.key_states[0] & 0x20)
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_FILE_SEEK), NULL);

							// F3 - View
							if (keyinfo.key_states[0] & 8)
							{
								if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
									((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_VIEW), NULL);
							}

							// F4 - Edit
							if (keyinfo.key_states[0] & 4)
							{
								if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
									((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_EDIT), NULL);
							}
	
							// F5 - MakeDir
							if (keyinfo.key_states[0] & 0x2)
							{
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_COPY), NULL);
							}
							
							// F6 - Move
							if (keyinfo.key_states[0] & 0x1)
							{
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_MOVE), NULL);
							}
	
							// F7 - MakeDir
							if (keyinfo.key_states[1] & 0x80)
							{
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_MAKEDIR), NULL);
							}
	
							// F8 - Delete
							if (keyinfo.key_states[1] & 0x40)
							{
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_DELETE), NULL);
							}
	
							// F10 - Quit
							if (keyinfo.key_states[1] & 0x10)
							{
								((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_QUIT), NULL);
							}
						}
					}
				}
				break;
			case B_DELETE:
				if (CountSelectedEntries(CT_WITHOUTPARENT)>0)		// Because the '..' cannot be deleted...
					((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_DELETE), NULL);
				break;
			case B_SPACE:
				if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
					((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_SPACE), NULL);
				break;
			case B_ENTER:
				if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
					((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_ENTER), NULL);
				process = false;
				break;
			case B_PAGE_UP:
				if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
				{
					int selected = CurrentSelection(0);
					int items = GetNumberOfVisibleItems();

					if (selected == 0) // a legelson vagyunk?
					{
						process = false;
						break;
					}
					
					Deselect(selected);

					// Last?
					if ((selected-items)>=0)
						Select(selected-items,false);
					else
						Select(0,false);
									
					ScrollToSelection();
				}
				process = false;
				break;
			case B_RIGHT_ARROW:
			case B_PAGE_DOWN:
				if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
				{
					int selected = CurrentSelection(0);
					int items = GetNumberOfVisibleItems();
					
					if (CountItems() == (selected +1))	// az utolson vagyunk?
					{
						process = false;
						break;
					}
					
					Deselect(selected);

					// Last?
					if ((selected+items)<CountItems())
						Select(selected+items,false);
					else
						Select(CountItems()-1,false);
									
					ScrollToSelection();
				}
				process = false;
				break;
			case B_INSERT:
				if (CountSelectedEntries(CT_WITHPARENT)>0)
				{
					CustomListItem *item = GetSelectedEntry(CountSelectedEntries(CT_WITHPARENT)-1);
					if (item)
						Select(IndexOf(item)+1,true);
				}
				else
					Select(0,true);				
				break;
			case B_LEFT_ARROW:
				Select(0,false);
				ScrollToSelection();
				break;
			default:
//				char buf[256];
//				sprintf(buf,"Byte:%d",bytes[i]);
//				BAlert *myAlert = new BAlert("DebugInfo",buf,"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
//				myAlert->Go();
				if (bytes[i]>=33 && bytes[i]<=122)
				{
					BMessage *message = new BMessage(MSG_ACTIVATE_COMMAND_LINE);
					message->AddInt8("Chr",bytes[i]);
					Window()->PostMessage(message, NULL);
				}
				break;
		}
	}	
		
	if (process)
		BListView::KeyDown(bytes,numBytes);
}

////////////////////////////////////////////////////////////////////////
void CustomListView::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	BListView::MessageReceived(message);
}

////////////////////////////////////////////////////////////////////////
int CustomListView::CountEntries(int mode)
////////////////////////////////////////////////////////////////////////
{
	int n = CountItems();
	
	if (mode == CT_WITHOUTPARENT)
	{
		if (n>0 && ((CustomListItem *)FirstItem())->m_Type==FT_PARENT) n--;
	}
	
	return n;
}

////////////////////////////////////////////////////////////////////////
int CustomListView::CountSelectedEntries(int mode)
////////////////////////////////////////////////////////////////////////
{	
	int32 selected;
	int i = 0;
	int c = 0;
	
	while ( (selected = CurrentSelection(i)) >= 0 )
	{
		if (mode == CT_WITHOUTPARENT)
		{
			if (((CustomListItem *)ItemAt(selected))->m_Type != FT_PARENT) c++;
		}
		else
			c++;
	
		i++;
	}
	
	return c;
}

////////////////////////////////////////////////////////////////////////
CustomListItem *CustomListView::GetSelectedEntry(int n)
////////////////////////////////////////////////////////////////////////
{
	int32 selected;

	selected = CurrentSelection(n);
	if (selected<0) return NULL;
	
	return (CustomListItem *)ItemAt(selected);
}

////////////////////////////////////////////////////////////////////////
void CustomListView::SetSelectionColor(uint8 red, uint8 green, uint8 blue)
////////////////////////////////////////////////////////////////////////
{
	SelectionColor.red = red;
	SelectionColor.green = green;
	SelectionColor.blue = blue;
	SelectionColor.alpha = 255;
}

////////////////////////////////////////////////////////////////////////
// Returns the total size of all files and directories
uint64 CustomListView::GetCurrentTotalSize()
////////////////////////////////////////////////////////////////////////
{
	uint64 n = 0;
	CustomListItem *item;

	for (int i=0;i<CountItems();i++)
	{
		item = (CustomListItem *)ItemAt(i);
		if (item)
		{
			if (item->m_Type == FT_FILE || item->m_Type == FT_DIRECTORY)
				n += item->m_FileSize;
		}
	}

	return n;
}

////////////////////////////////////////////////////////////////////////
uint64 CustomListView::GetSelectedTotalSize()
////////////////////////////////////////////////////////////////////////
{
	uint64 n = 0;
	int i=0;
	
	CustomListItem *item;
	
	while ((item = GetSelectedEntry(i)) != NULL)
	{
		if (item->m_Type == FT_FILE || item->m_Type == FT_DIRECTORY)
			n+=item->m_FileSize;
		i++;
	}
	
	return n;
}

////////////////////////////////////////////////////////////////////////
void CustomListView::MouseDown(BPoint point)
////////////////////////////////////////////////////////////////////////
{
	m_Now = system_time();
	
	if ((m_Now-m_LastClicked)<m_ClickSpeed)
	{
		if (m_LastClickedItem == IndexOf(point))
		{
			if (CountSelectedEntries(CT_WITHPARENT)==1)		// Because the '..' is also selectable...
				((PanelView *)Parent())->Looper()->PostMessage(new BMessage(MSG_ENTER), NULL);		

			// We have to delete the last click's details to be sure the next double-click will be OK...
			m_LastClickedItem = -1;
			m_LastClicked = 0;
		}
		else
		{
			m_LastClickedItem = IndexOf(point);
			m_LastClicked = m_Now;	
		}
	}
	else
	{
		m_LastClickedItem = IndexOf(point);
		m_LastClicked = m_Now;	
	}

	BListView::MouseDown(point);
}

////////////////////////////////////////////////////////////////////////
CustomListItem *CustomListView::FindItemByFileNamePath(BString filePath, BString fileName)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	int32 n = CountItems();

	for (int32 i=0;i<n;i++)
	{
		item = (CustomListItem *)ItemAt(i);
		if (item && item->m_FileName == fileName && item->m_FilePath == filePath)
			return item;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////
CustomListItem *CustomListView::FindItemByNodeRef(node_ref noderef)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	int32 n = CountItems();

	for (int32 i=0;i<n;i++)
	{
		item = (CustomListItem *)ItemAt(i);
		if (item && item->m_NodeRef == noderef)
			return item;
	}
	
	return NULL;
}

////////////////////////////////////////////////////////////////////////
void CustomListView::AddSortedItem(CustomListItem *item)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *tempitem;
	int32 first, last;
	int32 n = CountItems();

	if (n==0) AddItem(item);
	else
	{
		first = 0;
		last = n-1;
	
		while ((last-first)>0)
		{
			tempitem = (CustomListItem *)ItemAt((first + last) / 2);
			if (CompareFunc(item, tempitem)>=0)
				first = (first + last) / 2 + 1;
			else
				last = (first + last) / 2 - 1;
		}

		tempitem = (CustomListItem *)ItemAt(first);
		if (CompareFunc(item, tempitem)>=0)	
			AddItem(item,first + 1); // nagyobb -> utana
		else
			AddItem(item,first);	// kisebb -> elotte
	}
}

////////////////////////////////////////////////////////////////////////
int CustomListView::GetNumberOfVisibleItems(void)
////////////////////////////////////////////////////////////////////////
{
	float height = Bounds().bottom - Bounds().top;

	if (GetBoolSetting(SETTING_SHOWICON))
		return (int)height/16;
	else
		return (int)height/14;
}
