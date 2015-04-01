#include "GenesisMakeDirWindow.h"
#include "GenesisPanelView.h"
#include "GenesisWindow.h"
#include <stdio.h>
#include <View.h>
#include <Window.h>
#include <Beep.h>
#include <Button.h>
#include <Directory.h>

////////////////////////////////////////////////////////////////////////
GenesisMakeDirWindow::GenesisMakeDirWindow(const char* dirpath, BLooper* looper, BWindow *mainwindow) :
	BWindow(BRect(0,0,320,100), "Create new folder", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	m_DirPath.SetTo(dirpath);
	m_Looper = looper;

	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE);

	AddToSubset(mainwindow);

	m_View = new BView(Bounds(), "makedirview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewColor(216, 216, 216, 0);
	AddChild(m_View);

	// Bottom View	
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "infobottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);	//
	m_View->AddChild(BottomView);	
	
	// OK Button	
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;	
	m_OkButton = new BButton(rect,"ok","OK",new BMessage(BUTTON_MSG_CREATE_DIR),0,B_WILL_DRAW);
	BottomView->AddChild(m_OkButton);

	//Cancel Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;	
	BButton *CancelButton = new BButton(rect,"cancel","Cancel",new BMessage(BUTTON_MSG_CANCEL),0,B_WILL_DRAW);
	BottomView->AddChild(CancelButton);

	SetDefaultButton(m_OkButton);

	// Edit field
	rect = BottomView->Bounds();
	rect.top = rect.top+20;
	rect.bottom = rect.top+32;
	rect.left += 20;
	rect.right -= 20;
	m_DirName = new BTextControl( rect, "dirname", "Name:", "New folder", NULL );
	m_DirName->SetDivider(m_View->StringWidth("Name:")+4);
	m_DirName->SetModificationMessage(new BMessage(DIRNAME_CHANGED));
	m_View->AddChild(m_DirName);

	m_DirName->MakeFocus(true);	
	
	// Ctrl + Q closes the window...
	AddShortcut('Q', 0, new BMessage(BUTTON_MSG_CANCEL));
	
	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCEL)));

	if (strlen(m_DirName->Text())==0)
		m_OkButton->SetEnabled(false);
	else
		m_OkButton->SetEnabled(true);
	
	// If there is a given window, let's align our window to its center...
	if (mainwindow)
	{
		BRect myrect = Bounds();
		
		rect = mainwindow->Frame();
		float w = rect.right - rect.left;
		float h = rect.bottom - rect.top;
		MoveTo(rect.left + w/2 - (myrect.right-myrect.left)/2, rect.top + h/2 - (myrect.bottom-myrect.top)/2);
	}
}

////////////////////////////////////////////////////////////////////////
GenesisMakeDirWindow::~GenesisMakeDirWindow()
////////////////////////////////////////////////////////////////////////
{

}

////////////////////////////////////////////////////////////////////////
void GenesisMakeDirWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case DIRNAME_CHANGED:
			if (strlen(m_DirName->Text())>0)
				m_OkButton->SetEnabled(true);
			else
				m_OkButton->SetEnabled(false);
			break;
		case BUTTON_MSG_CANCEL:
			Close();
			break;
		case BUTTON_MSG_CREATE_DIR:
			if (strlen(m_DirName->Text())>0)
			{
				if ( CreateFolder(m_DirPath.String(), m_DirName->Text()) )
				{
					BMessage *msg = new BMessage(MSG_RELOAD);
					msg->AddString("ItemName",m_DirName->Text());
					if (m_Looper) m_Looper->PostMessage(msg, NULL);
					Close();
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
bool GenesisMakeDirWindow::CreateFolder(const char *dirpath, const char *dirname)
////////////////////////////////////////////////////////////////////////
{
	BString dir;
	dir.SetTo(dirpath);
	dir+="/";
	dir+=dirname;

	if (create_directory(dir.String(), 0777)==B_OK)		// TODO: jo a 0777?
		return 1;
	else
		return 0;
}




////////////////////////////////////////////////////////////////////////
CustomTextControl::CustomTextControl(BRect rect, const char *name, const char *label, const char *text) :
	BTextControl(rect, name, label, text, NULL)
////////////////////////////////////////////////////////////////////////
{


}

////////////////////////////////////////////////////////////////////////
CustomTextControl::~CustomTextControl()
////////////////////////////////////////////////////////////////////////
{

}
/*
void CustomTextControl::KeyUp(const char *bytes, int32 numBytes)
{
	if (bytes[0]==B_ESCAPE)
	{
		beep();
	}
	else BTextControl::KeyDown(bytes, numBytes);
}
*/
////////////////////////////////////////////////////////////////////////
void CustomTextControl::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	beep();
	switch(message->what)
	{
		default:
			BTextControl::MessageReceived(message);
	}
}

/*
void CustomTextControl::SetESCMessage(BMessage *msg)
{
	escmsg = msg;
}
*/
