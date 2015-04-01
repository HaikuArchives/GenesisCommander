#ifndef _GENESISGETINFOWINDOW_H_
#define _GENESISGETINFOWINDOW_H_

#include "GenesisCustomListItem.h"
#include <Window.h>
#include <Bitmap.h>
#include <Volume.h>
#include <Picture.h>

enum {
	ET_DIRECTORY,
	ET_SYMLINK,
	ET_FILE,
};

const uint32 BUTTON_MSG_OK	= 'BMOK';

class GenesisGetInfoWindow : public BWindow
{
	public:
		GenesisGetInfoWindow(const char* filename, BWindow *mainwindow = NULL);
		~GenesisGetInfoWindow();

		void ExamineDirectory(const char* filename);
		void ExamineSymLink(const char* filename);
		void ExamineFile(const char* filename);				
						
		virtual void	MessageReceived(BMessage* message);

		BView *m_IconView;
		BView *m_View;
};

class PieView : public BView
{
	public:
		PieView(BRect frame, char *name, off_t capacity, off_t free);
		~PieView();
		
		virtual void Draw(BRect r);
		
		off_t m_Capacity, m_Free;
};

class GenesisGetDiskInfoWindow : public BWindow
{
	public:
		GenesisGetDiskInfoWindow(CustomListItem *item, BWindow *mainwindow = NULL);
		~GenesisGetDiskInfoWindow();

		void ExamineDevice(BVolume *v);

		virtual void	MessageReceived(BMessage* message);

		BView *m_IconView;
		BView *m_View;

		PieView *m_PieView;
};

#endif