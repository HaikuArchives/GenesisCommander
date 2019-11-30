/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "Language.h"
#include "Settings.h"
#include <File.h>
#include <stdio.h>

Language *Language::m_Language = NULL;

////////////////////////////////////////////////////////////////////////
Language::Language(BString langfile)
////////////////////////////////////////////////////////////////////////
{
    m_Language = this;
	BString FullName;
	
	m_Text = NULL;
	m_Pos = 0;	

	FullName.SetTo(SETTINGS->GetAppPath());
	FullName << "/Languages/" << langfile;

	if (!LoadLanguageFile(FullName.String()))
		CreateDefaults(); // Ha nem sikerul betolteni a fajlt...
}

////////////////////////////////////////////////////////////////////////
Language::~Language()
////////////////////////////////////////////////////////////////////////
{
	LanguageEntry *tempentry;
	int n = m_Entries.CountItems();
	
	for (int i=0; i<n; i++)
	{
		tempentry = (LanguageEntry *)m_Entries.ItemAt(0);
		if (tempentry)
		{
			m_Entries.RemoveItem(tempentry);
			delete tempentry;
		}
	}
	
	if (m_Text)
	{
		delete m_Text;
		m_Text = NULL;
	}
	
	m_Language = NULL;
}

////////////////////////////////////////////////////////////////////////
void Language::CreateDefaults()
////////////////////////////////////////////////////////////////////////
{
	BString text;

	AddEntry("WINDOW_TITLE", "Genesis Commander");
	AddEntry("MENU_FILE", "File");
	AddEntry("SUBMENU_GETINFO", "Get info...");
	AddEntry("SUBMENU_GETNODEINFO", "Get node info...");
	AddEntry("SUBMENU_PREFERENCES", "Preferences...");
	AddEntry("SUBMENU_QUIT", "Quit");
	AddEntry("MENU_SELECTION", "Selection");
	AddEntry("SUBMENU_SELECTALL", "Select all");
	AddEntry("SUBMENU_DESELECTALL", "Deselect all");
	AddEntry("SUBMENU_INVERTSELECTION", "Invert selection");
	AddEntry("SUBMENU_ADDALLFOLDERS", "Add all folders to selection");
	AddEntry("SUBMENU_ADDALLFILES", "Add all files to selection");
	AddEntry("SUBMENU_ADDALLSYMLINKS", "Add all symlinks to selection");
	AddEntry("SUBMENU_SELECTGROUP", "Select group...");
	AddEntry("SUBMENU_DESELECTGROUP", "Deselect group...");
	AddEntry("SUBMENU_SEEKINLIST", "Seek in list");
	AddEntry("MENU_COMMANDS", "Commands");
	AddEntry("SUBMENU_VIEW", "View...");
	AddEntry("SUBMENU_EDIT", "Edit...");
	AddEntry("SUBMENU_EDITNEW", "Edit new...");
	AddEntry("SUBMENU_COPY", "Copy...");
	AddEntry("SUBMENU_MOVE", "Move...");
	AddEntry("SUBMENU_RENAME", "Rename...");
	AddEntry("SUBMENU_MAKEDIR", "Make dir...");
	AddEntry("SUBMENU_DELETE", "Delete...");
	AddEntry("SUBMENU_CREATELINK", "Create link on Desktop...");
	AddEntry("MENU_PANELS", "Panels");
	AddEntry("SUBMENU_RELOAD", "Reload current panel");
	AddEntry("SUBMENU_SWAP", "Swap panels");
	AddEntry("SUBMENU_TARGET_EQ_SOURCE", "Target = Source");
	AddEntry("MENU_SERVICES", "Services");
	AddEntry("SUBMENU_RESTARTINPUTSERVER", "Restart input server");
	AddEntry("SUBMENU_RESTARTMEDIASERVER", "Restart media server");
	AddEntry("SUBMENU_RESTARTNETWORKSERVER", "Restart network server");
	AddEntry("SUBMENU_TERMINAL", "Open terminal window");
	AddEntry("MENU_HELP", "Help");
	AddEntry("SUBMENU_ABOUT", "About...");
	
	AddEntry("BUTTON_F3", "F3 - View"); 
	AddEntry("BUTTON_F4", "F4 - Edit"); 
	AddEntry("BUTTON_F5", "F5 - Copy"); 
	AddEntry("BUTTON_F6", "F6 - Move"); 
	AddEntry("BUTTON_F7", "F7 - MkDir"); 
	AddEntry("BUTTON_F8", "F8 - Delete"); 
	AddEntry("BUTTON_F10", "F10 - Quit"); 
	
	AddEntry("QUIT", "Do you really want to quit?");
	
	text.SetTo("");
	text << "Genesis Commander for Haiku\n";
	text << "Version: <VER>\n";
	text << "Build time: <DATE> - <TIME>\n\n";
	text << "Programmed by: Zsolt Prievara\n\n";
	text << "This program is a beta version and there is no\n";
	text << "warranty. Use it only at your own risk!\n\n";
	text << "Send bug reports to: github.com/HaikuArchives/GenesisCommander/issues\n";
	AddEntry("ABOUT", text.String());
	
	AddEntry("CD_PARENT", "Parent (..)");
	AddEntry("CD_ROOT", "Root (/)");
	AddEntry("CD_HOME", "Home (<DIR>)");
	AddEntry("CD_DESKTOP", "Desktop (<DIR>)");	
	AddEntry("CD_DISKS", "Disks");
	
	AddEntry("PANELMENU_FIND", "Find...");
	AddEntry("PANELMENU_SHOWICONS", "Show icons");


	
	AddEntry("GENERAL_YES","Yes");
	AddEntry("GENERAL_NO","No");
	AddEntry("GENERAL_CANCEL","Cancel");
	AddEntry("GENERAL_ABORT","Abort");
	AddEntry("GENERAL_OK","OK");		
}

////////////////////////////////////////////////////////////////////////
void Language::AddEntry(BString key, BString value)
////////////////////////////////////////////////////////////////////////
{
	if (key.Length()==0)
		return;
	
	LanguageEntry *tempentry;
	tempentry = new LanguageEntry();
	if (tempentry)
	{
		tempentry->HashValue = GetHash(key);
		tempentry->Key = key;
		tempentry->Value = value;
		m_Entries.AddItem(tempentry);
	}
}

////////////////////////////////////////////////////////////////////////
BString Language::GetEntry(BString key)
////////////////////////////////////////////////////////////////////////
{
	if (key.Length() == 0)
		return BString("");
	
	unsigned char hashcode = GetHash(key);
	LanguageEntry *tempentry;

	for (int i=0; i<m_Entries.CountItems(); i++)
	{
		tempentry = (LanguageEntry *)m_Entries.ItemAt(i);
		if (tempentry)
		{
			if (tempentry->HashValue == hashcode && tempentry->Key == key)
				return tempentry->Value;
		}
	}	
	
	return BString("???");
}

////////////////////////////////////////////////////////////////////////
unsigned char Language::GetHash(BString text)
////////////////////////////////////////////////////////////////////////
{
	unsigned char result = 0;
	
	text.ToUpper();
	for (int i=0; i<text.Length(); i++)
		result ^= text.ByteAt(i);
		
	return result;
}

////////////////////////////////////////////////////////////////////////
bool Language::LoadLanguageFile(BString filename)
////////////////////////////////////////////////////////////////////////
{
	BString line;
	int	sign;		// itt van az '=' jel
	int len;
	BString key;
	BString value;

	if (Read(filename))
	{
		while (GetLine(line))
		{
			RemoveComment(line);
			sign = line.FindFirst('=');
			if (sign >= 1)
			{
				len = line.Length();
				line.CopyInto(key, 0, sign);
				line.CopyInto(value, sign+1, len-sign-1);
				Trim(key);
				Trim(value);
				AddEntry(key, value);
			}				
		}
	}
	else
		return false;
		
	return true;
}

////////////////////////////////////////////////////////////////////////
void Language::Trim(BString &result)
////////////////////////////////////////////////////////////////////////
{
	if (result.Length() == 0)
		return;

	// Elejen a space-ek...	
	while (result.ByteAt(0) == ' ')
	{
		result.Remove(0, 1);
	}
	
	if (result.Length() == 0)
		return;

	// Vegen a space-ek...
	while (result.ByteAt(result.Length()-1) == ' ')
	{
		result.Remove(result.Length()-1, 1);
	}
}

////////////////////////////////////////////////////////////////////////
bool Language::Read(BString filename)
////////////////////////////////////////////////////////////////////////
{
	ssize_t readsize;
	off_t size;
	
	m_File.SetTo(filename.String(), B_READ_ONLY);
	if (m_File.GetSize(&size) != B_OK)
		return false;

	if (size<=0)
		return false;

	m_Text = new char[size];
	if (m_Text == NULL)
		return false;

	readsize = m_File.Read(m_Text, size);
	
	if (readsize != size)
	{
		delete m_Text;
		return false;
	}

	m_Size = readsize;
	m_Pos = 0;
	return true;	
}

////////////////////////////////////////////////////////////////////////
bool Language::GetLine(BString &result)
////////////////////////////////////////////////////////////////////////
{
	char chr;
	int counter = 0;
	bool quit = false;

	if (m_Pos>=(m_Size-1))	// TODO: check
		return false;

	result.SetTo("");

	while (quit == false)
	{
		chr = m_Text[m_Pos+counter];
		if (chr == 10 || (m_Pos+counter>=m_Size))
			quit = true;
		else
			result << chr;
			
		counter++;
	}

	m_Pos += counter;	
	return true;	
}

////////////////////////////////////////////////////////////////////////
void Language::RemoveComment(BString &result)
////////////////////////////////////////////////////////////////////////
{
	int len = result.Length();
	int x = result.FindFirst('#');

	if (x>=0)
		result.Remove(x, len-x);
}
