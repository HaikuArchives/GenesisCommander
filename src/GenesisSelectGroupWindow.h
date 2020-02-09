/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019-2020 Ondrej Čerman
 */
#ifndef GENESISSELECTGROUPWINDOW_H
#define GENESISSELECTGROUPWINDOW_H

#include "GenesisCustomListView.h"

#include <String.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

const uint32 BUTTON_MSG_SELECT		= 'BSGN';
const uint32 BUTTON_MSG_CANCELSELECT= 'BSHC';
const uint32 ENTRYFILTER_CHANGED    = 'ENCH';

class GenesisSelectGroupWindow : public BWindow
{
	public:
		GenesisSelectGroupWindow(CustomListView *list, BWindow *mainwindow, bool selectmode = true);
		~GenesisSelectGroupWindow();

		CustomListView	*m_CustomListView;
		BWindow			*m_Window;
		BView 			*m_View;
		BButton 		*m_ActionButton;
		BTextControl	*m_EntryPattern;
		BLooper			*m_Looper;

		bool			m_SelectMode;

		virtual void	MessageReceived(BMessage* message);

	private:
		void 			ChangeSelection(BString input);
};

#endif
