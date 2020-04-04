/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019-2020, Ondrej Čerman
 */

#ifndef _GENESISAPP_H_
#define _GENESISAPP_H_

#include <Application.h>
#include <ObjectList.h>

#include "GenesisPreferencesWindow.h"
#include "GenesisWindow.h"

#define GENESIS_APP_SIG "application/x-vnd.GenesisCommander"

const uint32 MSG_MAINWIN_CLOSED			= 'MWCL';
const uint32 MSG_OPEN_PREFERENCES		= 'MOPR';

class GenesisApp: public BApplication
{
	public:
									GenesisApp();
		virtual void				ReadyToRun();
		virtual	void				MessageReceived(BMessage* message);
		virtual bool				QuitRequested();

		BString						m_AppName;
		int							GetMainWinCount();
		bool						QuitConfirmed();

	private:
		BObjectList<GenesisWindow>	*fMainWindowsList;
		GenesisPreferencesWindow	*fPrefWindow;
		bool						fQuitConfirmed;

		void						NewWindow();
		void						OpenPreferences(GenesisWindow *w);
};

#endif
