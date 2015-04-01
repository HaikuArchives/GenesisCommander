#ifndef _GENESISAPP_H_
#define _GENESISAPP_H_

#include <Application.h>

#include "GenesisWindow.h"

#define GENESIS_APP_SIG "application/x-vnd.GenesisCommander"

class GenesisApp: public BApplication
{
	public:
						GenesisApp();
		virtual void	ReadyToRun();
		BString			m_AppName;
	private:
		GenesisWindow*	fWindow;
};

#endif
