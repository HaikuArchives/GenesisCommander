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
	fMainWindowsList = new BObjectList<GenesisWindow>(5);
	fQuitConfirmed = false;
	fPrefWindow = NULL;
}

////////////////////////////////////////////////////////////////////////
void GenesisApp::NewWindow()
////////////////////////////////////////////////////////////////////////
{
	GenesisWindow* w;

	w = new GenesisWindow();
	w->Show();

	fMainWindowsList->AddItem(w);
}

////////////////////////////////////////////////////////////////////////
void GenesisApp::OpenPreferences(GenesisWindow *w = NULL)
////////////////////////////////////////////////////////////////////////
{
	if (fPrefWindow == NULL)
	{
		fPrefWindow = new GenesisPreferencesWindow();
		fPrefWindow->Show();
	}
	fPrefWindow->Activate();

	if (w != NULL)
	{
		fPrefWindow->CenterIn(w->Frame());
	}
}

////////////////////////////////////////////////////////////////////////
int GenesisApp::GetMainWinCount()
////////////////////////////////////////////////////////////////////////
{
	return fMainWindowsList->CountItems();
}

////////////////////////////////////////////////////////////////////////
bool GenesisApp::QuitConfirmed()
////////////////////////////////////////////////////////////////////////
{
	return fQuitConfirmed;
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
	if (fMainWindowsList->CountItems() > 0 && SETTINGS->GetAskOnExit())
	{
		BAlert *myAlert = new BAlert("Quit Genesis?","Do you really want to quit?","No","Quit",NULL,B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);

		if (myAlert->Go()!=1)
		{
			return false;
		}
	}
	fQuitConfirmed = true;

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
		case MSG_OPEN_PREFERENCES:
			{
				GenesisWindow* window;
				if (message->FindPointer("window", (void**) &window) == B_OK)
					OpenPreferences(window);
				else
					OpenPreferences();
			}
			break;
		case MSG_PREFERENCES_CHANGED:
			{
				int i;
				GenesisWindow* w;

				for (i = 0; i < fMainWindowsList->CountItems(); i++)
				{
					w = fMainWindowsList->ItemAt(i);
					if (w)
						w->PostMessage(MSG_PREFERENCES_CHANGED);
				}
			}
			break;
		case MSG_PREFERENCES_CLOSED:
			fPrefWindow = NULL;
			break;
		case MSG_MAINWIN_CLOSED:
			{
				GenesisWindow* window;
				if (message->FindPointer("window", (void**) &window) == B_OK)
					fMainWindowsList->RemoveItem(window);
			}
			break;

		default:
			BApplication::MessageReceived(message);
			break;
	}
}
