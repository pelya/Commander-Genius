/*
 * CPlayGameDialogs.cpp
 *
 *  Created on: 15.10.2009
 *      Author: gerstrong
 *
 *  This file is part of CPlayGame class and processes the dialogs like Statusscreen
 *  showhint Message, Game paused, etc.
 */

#include "CPlayGameVorticon.h"

void CPlayGameVorticon::processPauseDialogs()
{
	if(m_showPauseDialog)
	{
		showPausedGameDlg();
		return;
	}
	
	if(!mMessageBoxes.empty())
	{
		SmartPointer<CMessageBoxVort> pMB = mMessageBoxes.front();
		pMB->processLogic();

		if( pMB->isFinished() )
		{
			mMessageBoxes.pop_front();
		}

		return;
	}


}

void CPlayGameVorticon::showPausedGameDlg()
{
	// Open the Pause Dialog
	//drawTextMB("Game Paused");
}
