/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#ifndef _GENESISLANGUAGE_H_
#define _GENESISLANGUAGE_H_

#include <String.h>
#include <TextControl.h>
#include <List.h>
#include <File.h>

#define LANG(x) Language::m_Language->GetEntry(x)
#define LANGS(x) Language::m_Language->GetEntry(x).String()

struct LanguageEntry
{
	BString Key;
	BString Value;
	unsigned char HashValue;
};

class Language
{
public:
	Language(BString langfile);
	~Language();
	
	static Language *m_Language; 
	
	BString GetEntry(BString key);

private:
	void CreateDefaults();
	void AddEntry(BString key, BString value);
	unsigned char GetHash(BString text);
	bool LoadLanguageFile(BString filename);
	void Trim(BString &result);
	bool Read(BString filename);
	bool GetLine(BString &result);
	void RemoveComment(BString &result);
	
	BList	m_Entries;
	int   m_Pos;
	int   m_Size;
	BFile m_File;
	char *m_Text;
};


#endif
