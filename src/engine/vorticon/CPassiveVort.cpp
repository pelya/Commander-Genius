/*
 * CPassiveVort.cpp
 *
 *  Created on: 07.03.2010
 *      Author: gerstrong
 */

#include "CPassiveVort.h"

#include "graphics/CGfxEngine.h"
#include "common/CVorticonMapLoader.h"
#include "common/CTileProperties.h"
#include "sdl/CVideoDriver.h"
#include "sdl/input/CInput.h"
#include "sdl/extensions.h"
#include "core/mode/CGameMode.h"

namespace vorticon
{

bool CPassiveVort::init(char mode)
{
	m_mode = mode;


	SDL_Surface *temp = CG_CreateRGBSurface( g_pVideoDriver->getGameResolution().SDLRect() );
	mpTextSfc = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	createOutlinedText( 5, 170, "Press" );
	createOutlinedText( 5, 180, "Back(Esc)" );
	createOutlinedText( 5, 190, "for Menu" );


	if( m_mode == INTRO )
	{
		mpIntroScreen = new CIntro();
		mpMap = new CMap;
		CVorticonMapLoader MapLoader( mpMap );
		MapLoader.load( m_Episode, 90, m_DataDirectory);
		mpMap->gotoPos( 64+5*320, 32); // Coordinates of star sky
		mpMap->drawAll();
		mpIntroScreen->init();
		mpMap->changeTileArrayY(8, 15, 2, 560);
	}
	else if( m_mode == TITLE )
	{
		mpMap = new CMap;
		CVorticonMapLoader MapLoader( mpMap );
		MapLoader.load( m_Episode, 90, m_DataDirectory);
		mpMap->gotoPos( 32, 32 ); // Coordinates of title screen
		mpMap->drawAll();
		mpTitleScreen = new CTitle( *mpMap.get() );
		mpTitleScreen->init(m_Episode);
	}
	else if( m_mode == DEMO )
	{
		// TODO: Setup the demo environment
	}
	else
		return false;
	return true;
}

void CPassiveVort::process()
{
	// Process Drawing related stuff
	// Animate the tiles
	mpMap->animateAllTiles();

	// Blit the background
	g_pVideoDriver->mDrawTasks.add( new BlitScrollSurfaceTask() );

	CEventContainer& EventContainer = g_pBehaviorEngine->m_EventList;

	if(!EventContainer.empty())
	{
		if( EventContainer.occurredEvent<ResetScrollSurface>() )
		{
			g_pVideoDriver->updateScrollBuffer( mpMap );
			EventContainer.pop_Event();
			return;
		}
	}


	// Modes. We have three: Intro, Main-tile and Demos. We could add more.
	if( m_mode == INTRO )
	{
		// Intro code goes here!
		mpIntroScreen->process();

		if( mpIntroScreen->isFinished() )
		{
			// Shutdown mp_IntroScreen and show load Title Screen
			init(TITLE);
		}
	}
	else if( m_mode == TITLE )
	{
		mpTitleScreen->process();
	}
	else if( m_mode == DEMO )
	{
		// TODO: Demo modes are processed here!
		// TODO: Implement Demos here!
		init(TITLE);
	}

	g_pVideoDriver->mDrawTasks.add( new BlitSurfaceTask(mpTextSfc, NULL, NULL) );

}

}

