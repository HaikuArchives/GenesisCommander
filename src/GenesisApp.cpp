/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisApp.h"
#include "Settings.h"
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

}

////////////////////////////////////////////////////////////////////////
void GenesisApp::ReadyToRun()
////////////////////////////////////////////////////////////////////////
{
	fWindow = new GenesisWindow();
	fWindow->Show();
}
