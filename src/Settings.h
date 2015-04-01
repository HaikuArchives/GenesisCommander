#include <String.h>
#include <List.h>
#include <File.h>

#define SETTINGS Settings::m_Settings

#define SETTINGS_FN "Genesis_settings"

#define SETTING_INT(a)\
	private:\
		int a;\
	public:\
		int Get##a(void) { return a; }\
		void Set##a(int value) { a = value; m_SettingsChanged = true; }
		
#define SETTING_FLOAT(a)\
	private:\
		float a;\
	public:\
		float Get##a(void) { return a; }\
		void Set##a(float value) { a = value; m_SettingsChanged = true; }
		
#define SETTING_STRING(a)\
	private:\
		BString a;\
	public:\
		BString Get##a(void) { return a; }\
		void Set##a(BString value) { a = value; m_SettingsChanged = true; }\
		void Set##a(char *value) { a = value; m_SettingsChanged = true; }
		
#define SETTING_BOOL(a)\
	private:\
		bool a;\
	public:\
		bool Get##a(void) { return a; }\
		void Set##a(bool value) { a = value; m_SettingsChanged = true; }
		
class Settings
{
public:
	Settings();
	~Settings();
	
	static Settings *m_Settings;
	
	void SetDefaults(void);
	bool LoadSettings(void);
	bool SaveSettings(void);
	
	bool IsSettingsChanged(void) { return m_SettingsChanged; }
	
	Settings *Get(void) { return m_Settings; }

	// GLOBAL VARIABLES - Do not save these!
	SETTING_STRING(AppPath)
	SETTING_STRING(SettingsFile)

	// GENERAL SETTINGS
	SETTING_BOOL(AskOnExit)
	SETTING_INT(WindowLeft)
	SETTING_INT(WindowTop)
	SETTING_INT(WindowWidth)
	SETTING_INT(WindowHeight)
	SETTING_STRING(TerminalWindow)

	// LEFT PANEL SETTINGS
	SETTING_STRING(LeftPanelPath)
	
	// RIGHT PANEL SETTINGS
	SETTING_STRING(RightPanelPath)
	
	// LANGUAGE SETTINGS
	SETTING_STRING(Language)
	
private:
	BString GetFullSettingsFileName();
	bool m_SettingsChanged;
};

class DataFile
{
public:
	DataFile();
	~DataFile();

	bool LoadDataFile(BString filename);
	bool CreateDataFile(BString filename);
	
	bool GetInteger(BString key, int &result);
	bool GetString(BString key, BString &result);
	bool GetBool(BString key, bool &result);	
	
	void WriteInteger(BString key, int data);
	void WriteString(BString key, BString data);
	void WriteBool(BString key, bool data);
	void WriteText(BString text);
	
private:
	struct DataEntry
	{
		BString Key;
		BString Value;
		unsigned char HashValue;
	};

	bool GetEntry(BString key, BString &result);
	void AddEntry(BString key, BString value);
	unsigned char GetHash(BString text);
	void Trim(BString &result);
	bool Read(BString filename);
	bool GetLine(BString &result);
	void RemoveComment(BString &result);
	bool ToInt(BString text, int &result);
	
	BList	m_Entries;
	int   m_Pos;
	int   m_Size;
	BFile m_File;
	char *m_Text;
	BFile m_OutputFile;
};