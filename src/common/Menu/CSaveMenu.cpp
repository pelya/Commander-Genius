/*
 * CSaveMenu.cpp
 *
 *  Created on: 21.05.2010
 *      Author: gerstrong
 */

#include "CSaveMenu.h"
#include "CBaseMenu.h"
#include "CConfirmMenu.h"
#include "sdl/input/CInput.h"
#include "gui/CGUIInputText.h"
#include "fileio/CSaveGameController.h"
#include "common/CBehaviorEngine.h"
#include "common/Menu/CMenuController.h"

const std::string EMPTY_TEXT = "EMPTY";

CSaveMenu::CSaveMenu() :
CBaseMenu(CRect<float>(0.1f, 0.0f, 0.8f, 1.0f) ),
mp_OverwriteMenu(NULL),
m_overwrite(false)
{
	// Load the state-file list
	std::vector<std::string> StateFileList = gpSaveGameController->getSlotList();

	for(Uint32 i=1;i<=8;i++)
	{
		std::string text = EMPTY_TEXT;
		if(i <= StateFileList.size())
			text = StateFileList.at(i-1);

		mpMenuDialog->addControl(new CGUIInputText( text, CGUIInputText::VORTICON ) );
	}

	setMenuLabel("SAVEMENULABEL");
}

void CSaveMenu::sendEvent(SmartPointer<CEvent> command)
{
	// Before all events are sent to the dialog which handles selection catch some specific events
	// required for the saving process.
	if( CommandEvent *ev = dynamic_cast<CommandEvent*>(command.get()) )
	{
		const int sel = mpMenuDialog->Selection();
		if( sel > 0 )
		{
			if(ev->mCommand == IC_JUMP || ev->mCommand == IC_STATUS)
			{
				CGUIInputText *pInput = dynamic_cast<CGUIInputText*>(mpMenuDialog->CurrentControl());

				if(pInput->Typing())
				{
					gpSaveGameController->prepareSaveGame( sel, pInput->getText() );
					g_pBehaviorEngine->EventList().add( new CloseAllMenusEvent() );
				}
				else
				{
					if(pInput->getText() == EMPTY_TEXT)
						pInput->setText("");
					pInput->setTypeMode(true);
				}
				return;
			}
		}
	}

	mpMenuDialog->sendEvent(command);
}

