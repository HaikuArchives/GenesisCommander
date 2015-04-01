#include "Settings.h"
#include <Path.h>
#include <FindDirectory.h>
#include <stdio.h>

Settings *Settings::m_Settings = NULL;

////////////////////////////////////////////////////////////////////////
Settings::Settings()
////////////////////////////////////////////////////////////////////////
{
	m_Settings = this;
	SetSettingsFile(GetFullSettingsFileName());
	
	SetDefaults();
	if (!LoadSettings())	// ha nincs settings file...
	{
		if (!SaveSettings())
		{
			// ERROR - Cannot save settings file...
		}
	}
	
	m_SettingsChanged = false;
}

////////////////////////////////////////////////////////////////////////
Settings::~Settings()
////////////////////////////////////////////////////////////////////////
{
	m_Settings = NULL;
}

////////////////////////////////////////////////////////////////////////
void Settings::SetDefaults()
////////////////////////////////////////////////////////////////////////
{
	SetAskOnExit(false);
	SetLanguage("English");
	SetWindowLeft(100);
	SetWindowTop(100);
	SetWindowWidth(600);
	SetWindowHeight(400);
	SetLeftPanelPath("/");
	SetRightPanelPath("/");
	SetTerminalWindow("Genesis Terminal");
}

////////////////////////////////////////////////////////////////////////
bool Settings::LoadSettings()
////////////////////////////////////////////////////////////////////////
{
	#define GETSTRING(a,b) if (settingsfile.GetString(a, tempstring)) Set##b(tempstring);
	#define GETBOOL(a,b) if (settingsfile.GetBool(a, tempbool)) Set##b(tempbool);
	#define GETINT(a,b) if (settingsfile.GetInteger(a, tempint)) Set##b(tempint);

	BString tempstring;
	bool tempbool;
	int tempint;
	
	DataFile settingsfile;
	if (settingsfile.LoadDataFile(GetSettingsFile()))
	{
		if (settingsfile.GetBool("ASKONEXIT", tempbool))
			SetAskOnExit(tempbool);
		if (settingsfile.GetString("LANGUAGE", tempstring))
			SetLanguage(tempstring);
		if (settingsfile.GetInteger("WINDOWLEFT", tempint))
			SetWindowLeft(tempint);
		GETINT("WINDOWTOP", WindowTop);
		GETINT("WINDOWWIDTH", WindowWidth);
		GETINT("WINDOWHEIGHT", WindowHeight);
		GETSTRING("TERMINALWINDOW", TerminalWindow);
		
		GETSTRING("LEFTPANELPATH", LeftPanelPath);
		GETSTRING("RIGHTPANELPATH", RightPanelPath);
		
		return true;		
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
bool Settings::SaveSettings()
////////////////////////////////////////////////////////////////////////
{
	DataFile settingsfile;
	if (settingsfile.CreateDataFile(GetSettingsFile()))
	{
		settingsfile.WriteText(BString("############################\n"));
		settingsfile.WriteText(BString("# Genesis Commander settings file\n"));
		settingsfile.WriteText(BString("############################\n"));
		settingsfile.WriteText(BString("\n"));
		settingsfile.WriteText(BString("# General settings\n"));
		settingsfile.WriteString(BString("LANGUAGE"), GetLanguage());
		settingsfile.WriteBool(BString("ASKONEXIT"), GetAskOnExit());
		settingsfile.WriteInteger(BString("WINDOWLEFT"), GetWindowLeft());
		settingsfile.WriteInteger(BString("WINDOWTOP"), GetWindowTop());
		settingsfile.WriteInteger(BString("WINDOWWIDTH"), GetWindowWidth());
		settingsfile.WriteInteger(BString("WINDOWHEIGHT"), GetWindowHeight());
		settingsfile.WriteString(BString("TERMINALWINDOW"), GetTerminalWindow());
		settingsfile.WriteText(BString("\n"));
		settingsfile.WriteText(BString("# Left panel\n"));
		settingsfile.WriteString(BString("LEFTPANELPATH"), GetLeftPanelPath());
		settingsfile.WriteText(BString("\n"));
		settingsfile.WriteText(BString("# Right panel\n"));
		settingsfile.WriteString(BString("RIGHTPANELPATH"), GetRightPanelPath());
	
		return true;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
BString Settings::GetFullSettingsFileName()
////////////////////////////////////////////////////////////////////////
{
	BString result;
	BPath dirPath;
	
	find_directory(B_USER_SETTINGS_DIRECTORY, &dirPath, true);
	result.SetTo(dirPath.Path());

	result << "/" << SETTINGS_FN;	
	
	return result;	
}

/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
DataFile::DataFile()
////////////////////////////////////////////////////////////////////////
{
	m_Text = NULL;
	m_Pos = 0;	
}

////////////////////////////////////////////////////////////////////////
DataFile::~DataFile()
////////////////////////////////////////////////////////////////////////
{
	DataEntry *tempentry;
	int n = m_Entries.CountItems();
	
	for (int i=0; i<n; i++)
	{
		tempentry = (DataEntry *)m_Entries.ItemAt(0);
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
}

////////////////////////////////////////////////////////////////////////
bool DataFile::LoadDataFile(BString filename)
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
bool DataFile::CreateDataFile(BString filename)
////////////////////////////////////////////////////////////////////////
{	
	if (m_OutputFile.SetTo(filename.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE) != B_OK)
		return false;
	
	return true;	
}

////////////////////////////////////////////////////////////////////////
bool DataFile::GetInteger(BString key, int &result)
////////////////////////////////////////////////////////////////////////
{
	BString temp;

	if (GetEntry(key, temp))
	{
		if (ToInt(temp, result))
			return true;
		else
			return false;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
bool DataFile::GetString(BString key, BString &result)
////////////////////////////////////////////////////////////////////////
{
	BString temp;

	if (GetEntry(key, temp))
	{
		result = temp;
		return true;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
bool DataFile::GetBool(BString key, bool &result)
////////////////////////////////////////////////////////////////////////
{
	BString temp;

	if (GetEntry(key, temp))
	{
		temp.ToUpper();
		if (temp == "TRUE")
		{
			result = true;
			return true;
		}
		else if (temp == "FALSE")
		{
			result = false;
			return true;
		}
		
		return false;	// nem true es nem is false -> error es akkor default marad
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
void DataFile::WriteInteger(BString key, int data)
////////////////////////////////////////////////////////////////////////
{
	BString tempstring;
	tempstring.SetTo(key);
	tempstring << " = ";
	tempstring << data;
	tempstring << "\n";
	
	m_OutputFile.Write(tempstring.String(), tempstring.Length());
}

////////////////////////////////////////////////////////////////////////
void DataFile::WriteString(BString key, BString data)
////////////////////////////////////////////////////////////////////////
{
	BString tempstring;
	tempstring.SetTo(key);
	tempstring << " = ";
	tempstring << data;
	tempstring << "\n";
	
	m_OutputFile.Write(tempstring.String(), tempstring.Length());
}

////////////////////////////////////////////////////////////////////////
void DataFile::WriteBool(BString key, bool data)
////////////////////////////////////////////////////////////////////////
{
	BString tempstring;
	tempstring.SetTo(key);
	tempstring << " = ";
	if (data == true)
		tempstring << "true\n";
	else
		tempstring << "false\n";
	
	m_OutputFile.Write(tempstring.String(), tempstring.Length());
}

////////////////////////////////////////////////////////////////////////
void DataFile::WriteText(BString text)
////////////////////////////////////////////////////////////////////////
{
	m_OutputFile.Write(text.String(), text.Length());
}

////////////////////////////////////////////////////////////////////////
void DataFile::AddEntry(BString key, BString value)
////////////////////////////////////////////////////////////////////////
{
	if (key.Length()==0)
		return;
	
	DataEntry *tempentry;
	tempentry = new DataEntry();
	if (tempentry)
	{
		tempentry->HashValue = GetHash(key);
		tempentry->Key = key;
		tempentry->Value = value;
		m_Entries.AddItem(tempentry);
	}
}

////////////////////////////////////////////////////////////////////////
bool DataFile::GetEntry(BString key, BString &result)
////////////////////////////////////////////////////////////////////////
{
	if (key.Length() == 0)
		return false;
	
	unsigned char hashcode = GetHash(key);
	DataEntry *tempentry;

	for (int i=0; i<m_Entries.CountItems(); i++)
	{
		tempentry = (DataEntry *)m_Entries.ItemAt(i);
		if (tempentry)
		{
			if (tempentry->HashValue == hashcode && tempentry->Key == key)
			{
				result = tempentry->Value;
				return true;
			}
		}
	}	
	
	return false;
}

////////////////////////////////////////////////////////////////////////
unsigned char DataFile::GetHash(BString text)
////////////////////////////////////////////////////////////////////////
{
	unsigned char result = 0;
	
	text.ToUpper();
	for (int i=0; i<text.Length(); i++)
		result ^= text.ByteAt(i);
		
	return result;
}

////////////////////////////////////////////////////////////////////////
void DataFile::Trim(BString &result)
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
bool DataFile::Read(BString filename)
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
bool DataFile::GetLine(BString &result)
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
void DataFile::RemoveComment(BString &result)
////////////////////////////////////////////////////////////////////////
{
	int len = result.Length();
	int x = result.FindFirst('#');

	if (x>=0)
		result.Remove(x, len-x);
}

////////////////////////////////////////////////////////////////////////
bool DataFile::ToInt(BString text, int &result)
////////////////////////////////////////////////////////////////////////
{
	int n = text.Length();
	int num = 0;
	int decimal = 1;
	char x;

	if (n == 0)
		return false;
				
	for (int i=0; i<n; i++)
	{
		x = text[n-i-1];
		num = (x-0x30) * decimal;
		decimal *= 10;
	}

	result = num;
		
	return true;
}
