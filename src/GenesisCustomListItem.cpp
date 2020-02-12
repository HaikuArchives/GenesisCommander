/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#include "GenesisCustomListItem.h"
#include "GenesisCustomListView.h"
#include "GenesisPanelView.h"
#include "GenesisApp.h"
#include <stdio.h>
#include <stdlib.h>
#include <NodeInfo.h>
#include <Volume.h>
#include <Resources.h>
#include <Beep.h>
#include <Roster.h>
#include <Application.h>
#include <NodeMonitor.h>

////////////////////////////////////////////////////////////////////////
// Create as FILE
CustomListItem::CustomListItem(const char *filename, const char *filepath, int filetype, int filesize, BHandler *handler)
			: BListItem()
////////////////////////////////////////////////////////////////////////
{
	m_FileName.SetTo(filename);
	m_FilePath.SetTo(filepath);

	m_FileSize = filesize;
	m_Type = filetype;
	m_Handler = handler;

	m_IconImage = new BBitmap(BRect(0, 0, 15, 15), B_RGBA32);

	m_RSelected = false;

	// Generate node ref...
	BString fullnodepath(filepath);
	BNode node;

	fullnodepath << "/" << filename;
	node.SetTo(fullnodepath.String());
	node.GetNodeRef(&m_NodeRef);		// Store global ID to identify the entries...

	// Start watching current node...
	if (m_Handler)
		watch_node(&m_NodeRef, B_WATCH_STAT | B_WATCH_ATTR , m_Handler);
}

////////////////////////////////////////////////////////////////////////
// Create as DISK
CustomListItem::CustomListItem(const char *drivename, const char *diskpath, int filetype, off_t free, off_t capacity, dev_t deviceid)
			: BListItem()
////////////////////////////////////////////////////////////////////////
{
	m_FileName.SetTo(drivename);

	m_DiskPath.SetTo(diskpath);

	m_Type = filetype;
	m_FreeBytes = free;
	m_Capacity = capacity;
	m_DeviceID = deviceid;
	m_Handler = NULL;

	m_IconImage = new BBitmap(BRect(0, 0, 15, 15), B_CMAP8);

	m_RSelected = false;
}

////////////////////////////////////////////////////////////////////////
CustomListItem::~CustomListItem()
////////////////////////////////////////////////////////////////////////
{
	if (m_Handler) watch_node(&m_NodeRef, B_STOP_WATCHING, m_Handler);

	if (m_IconImage) delete m_IconImage;
}

////////////////////////////////////////////////////////////////////////
bool CustomListItem::GetIcon(const char *filename)
////////////////////////////////////////////////////////////////////////
{
	BNode node(filename);
	BNodeInfo nodeinfo(&node);

	// Let's get its icon...
	if (m_IconImage && (nodeinfo.InitCheck()==B_OK))
	{
		if (nodeinfo.GetTrackerIcon(m_IconImage,B_MINI_ICON)==B_OK)
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////
bool CustomListItem::GetIcon(BEntry *entry)
////////////////////////////////////////////////////////////////////////
{
	BNode node(entry);
	BNodeInfo nodeinfo(&node);

	// Let's get its icon...
	if (m_IconImage && (nodeinfo.InitCheck()==B_OK))
	{
		if (nodeinfo.GetTrackerIcon(m_IconImage,B_MINI_ICON)==B_OK)
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////
bool CustomListItem::GetIcon(BVolume *v)
////////////////////////////////////////////////////////////////////////
{
	if (m_IconImage && (v->InitCheck()==B_OK))
	{
		if (v->GetIcon(m_IconImage,B_MINI_ICON)==B_OK)
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////
void CustomListItem::AddIcon(unsigned char *data)
////////////////////////////////////////////////////////////////////////
{
	if (m_IconImage && data)
		m_IconImage->SetBits(data,256,0,B_CMAP8);
}

////////////////////////////////////////////////////////////////////////
void CustomListItem::DrawItem(BView *owner, BRect bounds, bool complete)
////////////////////////////////////////////////////////////////////////
{
	char buf[B_FILE_NAME_LENGTH+2];	// +2: []
	BFont be_italic_font(be_plain_font);
	be_italic_font.SetFace(B_ITALIC_FACE);
	int filesizewidth;			// Width of file size column...
	int filenamewidth;			// Width of file name column...
	int availablewidth;
	BString tempstring;

	bool Setting_ShowIcon = ((CustomListView *)owner)->GetBoolSetting(SETTING_SHOWICON);

	if (IsSelected() || complete)
	{
		rgb_color color;

		if (IsSelected())
			color = ((CustomListView *)owner)->SelectionColor;
		else
			color = owner->ViewColor();

		owner->SetHighColor(color);
		owner->FillRect(bounds);
	}

	switch (m_Type)
	{
		case FT_DISKITEM:
			owner->SetHighColor(0,64,0);
			break;
		case FT_PARENT:
		case FT_DIRECTORY:
			owner->SetHighColor(((CustomListView *)owner)->DirectoryColor);
			break;
		case FT_SYMLINKDIR:
		case FT_SYMLINKFILE:
		case FT_SYMLINKBROKEN:
			owner->SetHighColor(((CustomListView *)owner)->SymLinkColor);
			break;
		default:
			owner->SetHighColor(0,0,0);
			break;
	}

	if (IsSelected())
		owner->SetLowColor(((CustomListView *)owner)->SelectionColor);
	else
		owner->SetLowColor(owner->ViewColor());

	// Second row - File size...
	switch (m_Type)
	{
		case FT_SYMLINKFILE:
			sprintf(buf," ");
			break;
		case FT_FILE:
			if (m_FileSize>=(1024*1024))
				sprintf(buf,"%.02f MB",m_FileSize/(float)(1024*1024));
			else if (m_FileSize>=1024)
				sprintf(buf,"%.02f KB",m_FileSize/1024.0f);
			else
			{
				tempstring.SetTo("");
				tempstring << m_FileSize;
				sprintf(buf,"%s byte%s",tempstring.String(),m_FileSize<=1?"":"s");
			}
			break;
		case FT_SYMLINKBROKEN:
			sprintf(buf,"<Broken link>");
			break;
		case FT_PARENT:
		case FT_SYMLINKDIR:
		case FT_DIRECTORY:
			if (m_FileSize==0)
				sprintf(buf,"<DIR>");
			else
			{
				if (m_FileSize>=(1024*1024))
					sprintf(buf,"%.02f MB",m_FileSize/(float)(1024*1024));
				else if (m_FileSize>=1024)
					sprintf(buf,"%.02f KB",m_FileSize/1024.0f);
				else
				{
					tempstring.SetTo("");
					tempstring << m_FileSize;
					sprintf(buf,"%s byte%s",tempstring.String(),m_FileSize<=1?"":"s");
				}
			}
			break;
		case FT_DISKITEM:
			if (m_FreeBytes>=(1024*1024))
				sprintf(buf,"%.02f MB free",m_FreeBytes/(float)(1024*1024));
			else if (m_FreeBytes>=1024)
				sprintf(buf,"%.02f KB free",m_FreeBytes/1024.0f);
			else
			{
				tempstring.SetTo("");
				tempstring << m_FileSize;
				sprintf(buf,"%s byte%s free",tempstring.String(),m_FreeBytes<=1?"":"s");
			}
			break;
		default:
			sprintf(buf," ");
			break;
	}
	owner->SetFont(be_plain_font);	// Before any StringWidth we have to set the font!!!
	filesizewidth = (int)owner->StringWidth(buf);
	if (Setting_ShowIcon)
		owner->MovePenTo(bounds.right-4-filesizewidth, bounds.bottom-3);
	else
		owner->MovePenTo(bounds.right-4-filesizewidth, bounds.bottom-2);
	owner->DrawString(buf);

	// First row...
	if (Setting_ShowIcon)
		owner->MovePenTo(bounds.left+20, bounds.bottom-3);
	else
		owner->MovePenTo(bounds.left+4, bounds.bottom-2);

	switch (m_Type)
	{
		case FT_DISKBACK:
			sprintf(buf,"[Back]");
			owner->SetFont(be_plain_font);
			break;
		case FT_PARENT:
		case FT_DIRECTORY:
			sprintf(buf,"[%s]",m_FileName.String());
			owner->SetFont(be_plain_font);
			break;
		case FT_SYMLINKDIR:
			sprintf(buf,"[%s]",m_FileName.String());
			owner->SetFont(&be_italic_font);
			break;
		case FT_SYMLINKFILE:
		case FT_SYMLINKBROKEN:
			sprintf(buf,"%s",m_FileName.String());
			owner->SetFont(&be_italic_font);
			break;
		default:
			sprintf(buf,"%s",m_FileName.String());
			owner->SetFont(be_plain_font);
			break;
	}
	filenamewidth = (int)owner->StringWidth(buf);
	availablewidth = (int)(owner->Bounds().Width()) - filesizewidth - 2*4 - 8;	// last 8 is a space to have nicer look
	if (Setting_ShowIcon)
		availablewidth-=16;

	int len = strlen(buf);

	if (filenamewidth>availablewidth && ((len+3)<((int)sizeof(buf))))
	{
		buf[len+3]=0;
		buf[len+2]='.';
		buf[len+1]='.';
		buf[len  ]='.';

		while (filenamewidth>availablewidth)
		{
			len = strlen(buf);

			if (len-4<0) break;

			buf[len-1]=0;
			buf[len-2]='.';
			buf[len-3]='.';
			buf[len-4]='.';

			filenamewidth = (int)owner->StringWidth(buf);
		}
	}
	owner->DrawString(buf);

	// Icon
	if (Setting_ShowIcon && m_IconImage /*&& (m_Type!=FT_PARENT && m_Type!=FT_DISKBACK)*/)
	{
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(m_IconImage,BPoint(2, bounds.top));
	}
}

////////////////////////////////////////////////////////////////////////
void CustomListItem::Update(BView *owner, const BFont *font)
////////////////////////////////////////////////////////////////////////
{
	BListItem::Update(owner, font);
	bool Setting_ShowIcon = ((CustomListView *)owner)->GetBoolSetting(SETTING_SHOWICON);
	if (Setting_ShowIcon)
	{
		float iconheight = m_IconImage->Bounds().Height();
		if (Height() < iconheight)
		{
			SetHeight(iconheight);
		}
	}
}
