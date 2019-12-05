/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 *	2019, Ondrej Čerman
 */

#ifndef _GENESISPANELVIEW_H_
#define _GENESISPANELVIEW_H_

#include "GenesisCommandLine.h"
#include "GenesisCustomListView.h"
#include "GenesisCustomListItem.h"
#include <View.h>
#include <TextControl.h>
#include <Menu.h>
#include <MenuField.h>
#include <Box.h>
#include <StringView.h>
#include <ScrollView.h>
#include <Window.h>
#include <OS.h>
#include <MessageFilter.h>

const uint32 PATH_MSG_CD_PARENT 	= 'PPNT';
const uint32 PATH_MSG_CD_ROOT 		= 'PROT';
const uint32 PATH_MSG_CD_HOME 		= 'PHME';
const uint32 PATH_MSG_CD_DESKTOP 	= 'PDSK';
const uint32 PATH_MSG_CD_DISKS 		= 'PDIS';

const uint32 PANELMENU_MSG_FIND 	= 'PMFD';
const uint32 PANELMENU_MSG_SHOWICONS= 'PMSI';

const uint32 MSG_RELOAD 			= 'RELO';

const uint32 MSG_SEEKING			= 'SEEK';
const uint32 MSG_FILE_SEEK_END		= 'SEND';

enum {
	CR_DEFAULT,
	CR_HOURGLASS,
};

enum {
	PM_NORMAL,
	PM_DISKS,
	PM_FIND,
};

enum ADDTYPES {
	ADD_FILES,
	ADD_FOLDERS,
	ADD_SYMLINKS,
};

class PanelView : public BView
{
	public:
		PanelView(BRect frame, const char *name);
		~PanelView();

		// Hook functions...
		virtual void AttachedToWindow();
		virtual void MessageReceived(BMessage* message);
		virtual void FrameResized(float width, float height);

		CustomListView *GetList(void) { return m_CustomListView; }

		void SaveItemSelection(void);
		void LoadItemSelection(void);
		void Rescan(void);
		void RescanCreated(BEntry *entry);
		void RescanRemoved(node_ref nref);
		void RescanStat(node_ref nref);
		void RescanAttr(node_ref nref);
		void RescanForMissingEntries();
		void RescanForNewEntries();

		void EnableMonitoring(void);
		void DisableMonitoring(void);

//		virtual void SetMousePointer(int n);
		bool DoesEntryExist(const char *filename);
		void ChangePath(const char *p);
		void EnterDirectory(const char *p);
		void SelectAll(void);
		void DeselectAll(void);
		void InvertSelection(void);
		void AddToSelection(ADDTYPES type);
		void GotoParent(void);
		void GotoRoot(void);
		void GotoHome(void);
		void GotoDesktop(void);
		void GotoDisks(void);
		void Reload(void);
		void Reload(const char *itemname);	// reload and select given item...
		void Reload(int index);				// reload and select item by index...
		void DeleteDirectory(const char *dirname);
		void UniversalDelete(const char *filename);
	
		void LoadResources(void);
		
		void ClearFileList(void);
		void ReadDirectory(const char *itemname = NULL);
		CustomListItem *AddDirectoryEntry(BEntry *entry, bool sorted = false);
		void ReadDisks(void);

		void WriteFileSize(BString *str, uint64 FileSize);

		void SelectionChanged(void);
		void Execute(CustomListItem *item);
		void Calculate(CustomListItem *item);
		void SetPathStringView(void);
		void GetInfo(void);
		
		void SetPanelMode(int mode);

		uint64 GetDirectorySize(const char *path);
		void CreateLinkOnDesktop(void);

		void View(void);
		void Edit(void);
		void Edit(const char *filename);
		void Edit(CustomListItem *item);
		void Copy(void);
		void MakeDir(void);
		void MakeFile(bool edit);
		void Delete(void);
		void Rename(void);
		void Move(void);
		
		void SeekModeOn(void);
		void SeekModeOff(void);
		void SeekFor(const char *text);
		
		void SetPath(const char *path) { m_Path.SetTo(path); }

		BString			m_Path;				// This is the real path of the panel...
		BString			m_MonitoringPath;
		uint32			m_CurrentTotalSize;
		
		BMenuItem 		*m_CD_Parent;
		BMenuItem		*m_CD_Root;
		BMenuItem		*m_CD_Home;
		BMenuItem		*m_CD_Desktop;
		BMenuItem		*m_CD_Disks;
		
		BMenuItem 		*m_PanelMenu_Find;
		BMenuItem 		*m_PanelMenu_ShowIcons;
		
		BMenuField		*m_PathField;
		BMenuField		*m_PanelMenuField;
		BMenu			*m_PathMenu;
		BMenu			*m_PanelMenu;
		BBox			*m_Box;
		BStringView		*m_PathStringView;
		BStringView 	*m_StatusStringView;
		BBox			*m_Bevel_1;
		BBox			*m_Bevel_2;
		BTextControl	*m_SeekTextControl;

		CustomListView	*m_CustomListView;
		BScrollView 	*m_pScrollView;
		
		bigtime_t		m_LastSelectionTime;
		
		int				m_PanelMode;
		bool			m_SeekMode;
		
		// Resources
		unsigned char	*m_ParentIcon;
		unsigned char	*m_UnknownIcon;
				
		// Panel settings
		bool			m_Setting_ShowIcons;
		bool			m_Setting_ShowFileSize;
		bool			m_Setting_ShowFileDate;
};

#endif
