/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISCUSTOMLISTVIEW_H_
#define _GENESISCUSTOMLISTVIEW_H_

#include "GenesisCustomListItem.h"
#include <View.h>
#include <TextControl.h>
#include <StringView.h>
#include <ListView.h>

enum {
	CT_WITHPARENT,
	CT_WITHOUTPARENT,
};

enum SETTING {
	SETTING_SHOWICON = 0,
	SETTING_SHOWFILESIZE,
	SETTING_SHOWFILEDATE,
};

const uint32 MSG_ENTER 			= 'MENT';
const uint32 MSG_SPACE			= 'MSPC';
const uint32 MSG_VIEW			= 'MVIW';
const uint32 MSG_EDIT			= 'MVED';
const uint32 MSG_COPY			= 'MCPY';
const uint32 MSG_MOVE			= 'MMOV';
const uint32 MSG_RENAME			= 'MRNM';
const uint32 MSG_MAKEDIR		= 'MMKD';
const uint32 MSG_DELETE			= 'MDEL';
const uint32 MSG_QUIT			= 'MQUT';
const uint32 MSG_FILE_SEEK		= 'MPOP';

class CustomListView : public BListView
{
	public:
		CustomListView(BRect frame, const char *name, void *pv);
		~CustomListView();

		// Hook functions...
//		virtual void FrameResized(float width, float height);
		virtual void AttachedToWindow();
//		virtual void Draw(BRect r);
		virtual void KeyDown(const char *bytes, int32 numBytes);
//		virtual void SelectionChanged();
		virtual void MakeFocus(bool focusState = true);
		virtual void MouseDown(BPoint point);
		virtual void MessageReceived(BMessage* message);
				
		void DoSortList(void);
		
		int  CountEntries(int mode);
		int  CountSelectedEntries(int mode);
		CustomListItem *GetSelectedEntry(int n);
		void SetSelectionColor(uint8 red, uint8 green, uint8 blue);
		uint64 GetCurrentTotalSize(void);
		uint64 GetSelectedTotalSize(void);
		CustomListItem *FindItemByFileNamePath(BString filePath, BString fileName);
		CustomListItem *FindItemByNodeRef(node_ref noderef);
		void AddSortedItem(CustomListItem *item);
		int GetNumberOfVisibleItems(void);

		bool GetBoolSetting(SETTING n);

		// Double-click
	    bigtime_t	m_Now,m_LastClicked;
    	bigtime_t	m_ClickSpeed;
		int			m_LastClickedItem;
		
		rgb_color 	SelectionColor;
		rgb_color	DirectoryColor;
		rgb_color	SymLinkColor;
		
		void		*m_PV;

	private:
		static int CompareFunc( const void *first, const void *second);
		static int CompareFunc( CustomListItem *first, CustomListItem *second);
};

#endif
