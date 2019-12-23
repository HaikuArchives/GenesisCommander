/*
 * Copyright 2002-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2002-2004, Zsolt Prievara
 */

#include "GenesisRenameWindow.h"
#include "GenesisPanelView.h"
#include "GenesisWindow.h"
#include <stdio.h>
#include <View.h>
#include <Window.h>
#include <Alert.h>
#include <Button.h>
#include <Directory.h>
#include <Path.h>
#include <Beep.h>

////////////////////////////////////////////////////////////////////////
GenesisRenameWindow::GenesisRenameWindow(CustomListView *list, BWindow *mainwindow) :
	BWindow(BRect(0,0,320,100), "Rename", B_TITLED_WINDOW , B_WILL_DRAW)
////////////////////////////////////////////////////////////////////////
{
	BRect rect;

	m_CustomListView = list;
	m_Window = mainwindow;
	m_DoSort = false;
	m_ItemToSelect = NULL;
	m_ItemNameToSelect.SetTo("");

	((PanelView *)m_CustomListView->m_PV)->DisableMonitoring();
	
	SetType(B_FLOATING_WINDOW);
	SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
	SetFlags(B_NOT_RESIZABLE | B_NOT_ZOOMABLE);

	AddToSubset(mainwindow);

	RemoveParentSelection();

	m_View = new BView(Bounds(), "renameview", B_FOLLOW_ALL, B_WILL_DRAW);
	m_View->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	AddChild(m_View);

	// Bottom View	
	rect = Bounds();
	rect.top = rect.bottom-44;
	BView *BottomView = new BView(rect, "renamebottomview", B_FOLLOW_ALL, B_WILL_DRAW);
	BottomView->SetViewColor(180, 190, 200, 0);
	m_View->AddChild(BottomView);	
	
	// Rename Button	
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-80;
	rect.right = rect.right-20;	
	m_RenameButton = new BButton(rect,"rename","Rename",new BMessage(BUTTON_MSG_RENAME),0,B_WILL_DRAW);
	BottomView->AddChild(m_RenameButton);

	// Cancel Button
	rect = BottomView->Bounds();
	rect.top = rect.bottom-34;
	rect.bottom = rect.bottom-14;
	rect.left = rect.right-160;
	rect.right = rect.right-100;	
	BButton *CancelButton = new BButton(rect,"cancel","Cancel",new BMessage(BUTTON_MSG_CANCELRENAME),0,B_WILL_DRAW);
	BottomView->AddChild(CancelButton);

	SetDefaultButton(m_RenameButton);

	// Edit field
	rect = BottomView->Bounds();
	rect.top = rect.top+20;
	rect.bottom = rect.top+32;
	rect.left += 20;
	rect.right -= 20;
	m_EntryName = new BTextControl( rect, "entryname", "New name:", "", NULL );
	m_EntryName->SetDivider(m_View->StringWidth("New name:")+4);
	m_EntryName->SetModificationMessage(new BMessage(ENTRYNAME_CHANGED));
	m_View->AddChild(m_EntryName);

	m_EntryName->MakeFocus(true);	
	
	// Ctrl + Q closes the window...
	AddShortcut('Q', 0, new BMessage(BUTTON_MSG_CANCELRENAME));
	
	AddCommonFilter(new EscapeFilter(this, new BMessage(BUTTON_MSG_CANCELRENAME)));

	if (strlen(m_EntryName->Text())==0)
		m_RenameButton->SetEnabled(false);
	else
		m_RenameButton->SetEnabled(true);
	
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
GenesisRenameWindow::~GenesisRenameWindow()
////////////////////////////////////////////////////////////////////////
{
	if (m_ItemNameToSelect.Length()>0)
	{
		CustomListItem *item;
		bool found = false;

		m_Window->Lock();
		int n = m_CustomListView->CountItems();
		for(int i=0;i<n;i++)
		{
			item = (CustomListItem *)m_CustomListView->ItemAt(i);
			if (item)
			{
				if (item->m_FileName == m_ItemNameToSelect)
				{
					m_CustomListView->Select(i,false);
					found = true;
					break;
				}
			}
		}
		
		if (!found)
			m_CustomListView->Select(0,false);
		
		m_Window->Unlock();			
	}

	((PanelView *)m_CustomListView->m_PV)->EnableMonitoring();
}

////////////////////////////////////////////////////////////////////////
void GenesisRenameWindow::MessageReceived(BMessage* message)
////////////////////////////////////////////////////////////////////////
{
	switch(message->what)
	{
		case ENTRYNAME_CHANGED:
			if (strlen(m_EntryName->Text())>0)
				m_RenameButton->SetEnabled(true);
			else
				m_RenameButton->SetEnabled(false);
			break;
		case BUTTON_MSG_CANCELRENAME:
			Close();
			break;
		case BUTTON_MSG_RENAME:
			if (strlen(m_EntryName->Text())>0)
			{
				CustomListItem *item;

				item = m_CustomListView->GetSelectedEntry(0);
				if (item)
				{
					BString filename;
					BEntry entry;
					bool Ok = false;
					
					filename.SetTo(item->m_FilePath.String());
					filename << "/" << item->m_FileName;
				
					entry.SetTo(filename.String());
					if (entry.InitCheck()==B_OK)
					{
						if (entry.Exists()) // Letezik meg? :-)
						{
							if (strcmp(m_EntryName->Text(), item->m_FileName.String()) != 0) // Ha meg akarja valtoztatni a nevet...
							{								
								if (entry.Rename(m_EntryName->Text())==B_OK) // Sikerult atnevezni...
								{
									m_Window->Lock();
									((PanelView *)m_CustomListView->m_PV)->Rescan();
									m_Window->Unlock();
									m_ItemNameToSelect.SetTo(m_EntryName->Text());
									Ok = true;	// GetNext!
								}
								else	// Nem sikerult atnevezni...
								{
									BString text;
								
									text << "Cannot rename '" << item->m_FileName.String() << "' to '" << m_EntryName->Text() << "'.";
								
									BAlert *myAlert = new BAlert("Rename",text.String(),"Abort","Skip","Retry",B_WIDTH_AS_USUAL,B_OFFSET_SPACING,B_WARNING_ALERT);
									myAlert->SetShortcut(0, B_ESCAPE);
									switch (myAlert->Go())
									{
										case 0:	// Abort
											Close();
											break;
										case 1:	// Skip
											Ok = true;
											break;
										case 2:
											break;
									}							
								}
							}
							else	// Nem valtozott a neve, igy felesleges nevezgetni...
							{	
								m_Window->Lock();
								m_CustomListView->Deselect(m_CustomListView->IndexOf(item));
								m_CustomListView->InvalidateItem(m_CustomListView->IndexOf(item));
								m_Window->Unlock();
								m_ItemToSelect = item;
								Ok = true;	// Gyerunk a kovetkezore...
							}
						}
					}
				
					if (Ok)
					{
						GetNext();
					}
				}
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}

////////////////////////////////////////////////////////////////////////
void GenesisRenameWindow::Go(void)
////////////////////////////////////////////////////////////////////////
{
	Show();
	GetNext();
}

////////////////////////////////////////////////////////////////////////
void GenesisRenameWindow::GetNext(void)
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;

	item = m_CustomListView->GetSelectedEntry(0);
	if (item)
	{
		Lock();
		m_EntryName->SetText(item->m_FileName.String());		
		m_EntryName->MakeFocus(true);
		Unlock();
	}
	else
		Close();
}

////////////////////////////////////////////////////////////////////////
void GenesisRenameWindow::RemoveParentSelection()
////////////////////////////////////////////////////////////////////////
{
	CustomListItem *item;
	item = m_CustomListView->GetSelectedEntry(0);
	if (item && item->m_Type==FT_PARENT)
	{
		m_Window->Lock();
		m_CustomListView->Deselect(0);
		m_CustomListView->InvalidateItem(0);
		m_Window->Unlock();
	}
}

