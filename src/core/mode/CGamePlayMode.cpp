/*
 * CGamePlayMode.cpp
 *
 *  Created on: 26.03.2011
 *      Author: gerstrong
 */

#include "CGamePlayMode.h"
#include "engine/galaxy/CPlayGameGalaxy.h"
#include "engine/vorticon/playgame/CPlayGameVorticon.h"
#include "graphics/effects/CColorMerge.h"

CGamePlayMode::CGamePlayMode(const int Episode, const int Numplayers,
		const std::string& DataDirectory,
		const int startLevel) :
m_startLevel(startLevel),
m_Episode(Episode),
m_Numplayers(Numplayers),
m_DataDirectory(DataDirectory)
{}

void CGamePlayMode::init()
{
	CExeFile &ExeFile = g_pBehaviorEngine->m_ExeFile;
	CEventContainer& EventContainer = g_pBehaviorEngine->m_EventList;

	// If no level has been set or is out of bound, set it to map.
	if(m_startLevel > 100 || m_startLevel < 0 )
		m_startLevel = WORLD_MAP_LEVEL_VORTICON;

	bool ok = true;

	if(m_Episode >= 4)
	{
		if(m_startLevel == WORLD_MAP_LEVEL_VORTICON)
			m_startLevel = WORLD_MAP_LEVEL_GALAXY;
		mp_PlayGame = new galaxy::CPlayGameGalaxy( ExeFile, m_startLevel,
												m_Numplayers, m_SavedGame);
	}
	else
	{
		if(m_startLevel == WORLD_MAP_LEVEL_GALAXY)
			m_startLevel = WORLD_MAP_LEVEL_VORTICON;
		mp_PlayGame = new CPlayGameVorticon( ExeFile, m_startLevel,
											m_Numplayers, m_SavedGame);
	}

	// Create the special merge effect (Fadeout)
	CColorMerge *pColorMergeFX = new CColorMerge(8);

	ok &= mp_PlayGame->init();

	g_pGfxEngine->setupEffect(pColorMergeFX);


	if(!ok)
	{
		EventContainer.add( new GMSwitchToPassiveMode(m_DataDirectory, m_Episode));
	}
}

void CGamePlayMode::loadGame()
{
	mp_PlayGame->process();
	mp_PlayGame->loadGameState();
}


void CGamePlayMode::process()
{
	// The player is playing the game. It also includes scenes like ending
	CEventContainer& EventContainer = g_pBehaviorEngine->m_EventList;

	mp_PlayGame->process();

	if( EventContainer.occurredEvent<SaveGameEvent>() )
	{
		mp_PlayGame->saveGameState();
		EventContainer.pop_Event();
	}


	if( mp_PlayGame->getEndGame() )
	{
		m_startLevel = 0;
		EventContainer.add( new GMSwitchToPassiveMode(m_DataDirectory, m_Episode) );
	}
	else if( mp_PlayGame->getStartGame() )
	{ // Start another new game
		m_Numplayers = g_pBehaviorEngine->mPlayers;
		EventContainer.add( new GMSwitchToPlayGameMode(m_Episode, m_Numplayers, m_DataDirectory) );
	}
	else if( mp_PlayGame->getExitEvent() )
	{
		EventContainer.add( new GMQuit() );
	}

}
