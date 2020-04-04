/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019-2020, Ondrej Čerman
 */

#include "GenesisApp.h"
#include "Settings.h"
#include <Alert.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
////////////////////////////////////////////////////////////////////////
{
	// Ez elvileg nem fordulhat elo, de sosem tudni...
	if (argc<1)
		return(0);

	// Settings singleton letrehozasa...
	Settings* settings = new Settings();

	// Beallitjuk, hogy hol van az ELF...
	BString apppath(argv[0]);
	int x = apppath.FindLast("/");
	apppath.Remove(x, apppath.Length()-x);
	SETTINGS->SetAppPath(apppath);

	GenesisApp* app = new GenesisApp();
	app->Run();

	if (settings)
		delete settings;

	return(0);
}

////////////////////////////////////////////////////////////////////////
GenesisApp::GenesisApp()
////////////////////////////////////////////////////////////////////////
	: BApplication(GENESIS_APP_SIG)
{
	m_MainWinCount = 0;
	m_QuitConfirmed = false;
}

////////////////////////////////////////////////////////////////////////
void GenesisApp::NewWindow()
////////////////////////////////////////////////////////////////////////
{
	GenesisWindow* w;

	m_MainWinCount++;

	w = new GenesisWindow();
	w->Show();
}

////////////////////////////////////////////////////////////////////////
int GenesisApp::GetMainWinCount()
////////////////////////////////////////////////////////////////////////
{
	return m_MainWinCount;
}

////////////////////////////////////////////////////////////////////////
bool GenesisApp::QuitConfirmed()
////////////////////////////////////////////////////////////////////////
{
	return m_QuitConfirmed;
}

////////////////////////////////////////////////////////////////////////
void GenesisApp::ReadyToRun()
////////////////////////////////////////////////////////////////////////
{
	NewWindow();
}

////////////////////////////////////////////////////////////////////////
bool GenesisApp::QuitRequested()
////////////////////////////////////////////////////////////////////////
{
	if (m_MainWinCount > 0 && SETTINGS->GetAskOnExit())
	{
		BAlert *myAlert = new BAlert("Quit Genesis?","Do you really want to quit?","No","Quit",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);

		if (myAlert->Go()!=1)
		{
			return false;
		}
	}
	m_QuitConfirmed = true;

	return BApplication::QuitRequested();
}


////////////////////////////////////////////////////////////////////////
void GenesisApp::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch (message->what) {
		case B_SILENT_RELAUNCH:
			NewWindow();
			break;
		case MSG_MAINWIN_CLOSED:
			m_MainWinCount--;
			break;

		default:
			BApplication::MessageReceived(message);
			break;
	}
}
