/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISCUSTOMLISTITEM_H_
#define _GENESISCUSTOMLISTITEM_H_

#include <View.h>
#include <String.h>
#include <ListItem.h>
#include <Volume.h>
#include <Bitmap.h>

enum FILE_TYPE {
	FT_PARENT,
	FT_DIRECTORY,
	FT_SYMLINKFILE,
	FT_SYMLINKDIR,
	FT_SYMLINKBROKEN,
	FT_FILE,
	FT_FINDITEM,
	FT_FINDBACK,
	FT_DISKITEM,
	FT_DISKBACK,
};

class CustomListItem : public BListItem
{
	public:
		CustomListItem(const char *filename, const char *filepath, int filetype, int filesize, BHandler *handler = NULL);
		CustomListItem(const char *drivename, const char *diskpath, int filetype, off_t free, off_t capacity, dev_t deviceid);
		~CustomListItem();

		bool GetIcon(const char *filename);
		bool GetIcon(BEntry *entry);
		bool GetIcon(BVolume *v);
		void AddIcon(unsigned char *data);

		virtual void DrawItem(BView *owner,	BRect bounds, bool complete = false);
		virtual void Update(BView *owner, const BFont *font);

		BBitmap		*m_IconImage;
		bool		m_RSelected;		// Selected flag for Rescan
		node_ref 	m_NodeRef;			// A fajlok beazonositasara
		BHandler 	*m_Handler;

		// File entry
		BString		m_FileName;
		BString		m_FilePath;
		int			m_Type;			// enum!
		uint64		m_FileSize;

		// Disk entry
		BString		m_DiskName;		// TODO: ezt kellene hasznalni m_FileName helyett...
		BString		m_DiskPath;
		off_t		m_FreeBytes;	// Disks
		off_t		m_Capacity;		// Disks
		dev_t		m_DeviceID;
};

#endif
