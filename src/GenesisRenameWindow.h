/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISRENAMEWINDOW_H_
#define _GENESISRENAMEWINDOW_H_

#include "GenesisCustomListView.h"

#include <Window.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <String.h>

const uint32 BUTTON_MSG_RENAME		= 'BREN';
const uint32 BUTTON_MSG_CANCELRENAME= 'BREC';
const uint32 ENTRYNAME_CHANGED		= 'ENCH';

class GenesisRenameWindow : public BWindow
{
	public:
		GenesisRenameWindow(CustomListView *list, BWindow *mainwindow);
		~GenesisRenameWindow();

		CustomListView	*m_CustomListView;
		BWindow			*m_Window;
		BView 			*m_View;
		BButton 		*m_RenameButton;
		BTextControl	*m_EntryName;
		bool			m_DoSort;
		BLooper			*m_Looper;
		CustomListItem	*m_ItemToSelect;
		BString			m_ItemNameToSelect;

		virtual void	MessageReceived(BMessage* message);

		void 	Go(void);
		void	GetNext(void);
		void 	RemoveParentSelection();
};

#endif
