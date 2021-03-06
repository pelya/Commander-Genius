/*
 * CPlayGameWorldMap.cpp
 *
 *  Created on: 05.11.2009
 *      Author: gerstrong
 */

#include "CPlayGameVorticon.h"
#include "sdl/CTimer.h"
#include "sdl/input/CInput.h"
#include "sdl/sound/CSound.h"
#include "sdl/CVideoDriver.h"
#include "graphics/CGfxEngine.h"
#include "graphics/effects/CColorMerge.h"
#include "sdl/sound/CSound.h"
#include "engine/vorticon/ai/CTeleporter.h"
#include "engine/vorticon/ai/CMessie.h"

const int LVLS_SHIP = 20;

void CPlayGameVorticon::processOnWorldMap()
{
	int useobject;

	for( int i=0 ; i<m_NumPlayers ; i++ )
	{
		CPlayer &player = m_Player[i];

		// Perform player Objects...
		player.processWorldMap();

		// entered a level, used ship, teleporter, etc.
		if( !player.hideplayer && !player.beingteleported )
		{
			useobject = player.getNewObject();
			if( useobject != 0 )
			{
				// If it is teleporter, make the Player teleport
				int TeleportID;
				if( (TeleportID = getTeleporterInfo(useobject)) != 0 )
				{
					teleportPlayer(TeleportID, player);
				}
				else
				{
					// If it is level, change the playgame mode and load the new map. Nessie is
					// a special case in Episode 3
					switch(useobject)
					{
					case NESSIE_WEED:
					case NESSIE_PATH: break;
					case NESSIE_LAND:
						player.MountNessieIfAvailable();
						g_pInput->flushAll();
						break;

					case LVLS_SHIP:
					{
						if (m_Episode==1)
							YourShipNeedsTheseParts();
						else if (m_Episode==3)
							ShipEp3();

						g_pInput->flushCommands();

					}
					break;

					default: // a regular level
						// Check if Level has been completed or the Level-Replayability is enabled
						if( !mp_level_completed[useobject & 0x7fff] || mp_option[OPT_LVLREPLAYABILITY].value )
						{
							// Create the special merge effect
							CColorMerge *pColorMergeFX = new CColorMerge(8);

							m_level_command = START_LEVEL;
							m_Level = useobject & 0x7fff;
							g_pMusicPlayer->stop();
							player.playSound(SOUND_ENTER_LEVEL);
							// save where on the map, the player entered. This is a checkpoint!
							m_checkpoint_x = player.getXPosition();
							m_checkpoint_y = player.getYPosition();
							m_checkpointset = true;
							cleanup();
							init();

							g_pGfxEngine->setupEffect(pColorMergeFX);
						}
						break;
					}
				}
			}
		}

		if(player.mounted)
		{
			if(g_pInput->getPressedAnyCommand(0))
			{
				player.UnmountNessie();
				g_pInput->flushAll();
			}
		}

	}
}

void CPlayGameVorticon::goBacktoMap()
{
	// Create the special merge effect (Fadeout)
	CColorMerge *pColorMergeFX = new CColorMerge(8);

	// before he can go back to map, he must tie up the objects.
	// This means, all objects except the puppy ones of the player....
	m_Object.clear();

	m_level_command = START_LEVEL;
	m_Level = WM_MAP_NUM;
	g_pMusicPlayer->stop();
	// Now that the new level/map will be loaded, the players aren't dead anymore!
	std::vector<CPlayer>::iterator player= m_Player.begin();
	for( ; player != m_Player.end() ; player++ )
	{
		player->level_done = LEVEL_NOT_DONE;
		player->mHealthPoints = 1;

		// Restore checkpoint
		player->moveToForce(m_checkpoint_x,m_checkpoint_y);
		player->inventory.HasCardYellow = 0;
		player->inventory.HasCardBlue = 0;
		player->inventory.HasCardGreen = 0;
		player->inventory.HasCardRed = 0;


		// Now, that the level is complete, sprite can be shown again, and now goto map!
		int width = player->w>>(CSF-4);

		if(width > 0)
		{
			int frame = player->playerbaseframe;
			if(g_pBehaviorEngine->getEpisode() == 3) frame++;

			g_pGfxEngine->getSprite(frame+0).setWidth(width);
			g_pGfxEngine->getSprite(frame+1).setWidth(width);
			g_pGfxEngine->getSprite(frame+2).setWidth(width);
			g_pGfxEngine->getSprite(frame+3).setWidth(width);
		}


	}
	cleanup();
	init();

	// Second Snapshot for merge
	g_pGfxEngine->setupEffect(pColorMergeFX);
}

void CPlayGameVorticon::YourShipNeedsTheseParts()
{
	CMessageBoxVort *MessageBox = new CMessageBoxVort(g_pBehaviorEngine->getString("EP1_SHIP"));

	bool joy, bat, vac, wis;
	joy = bat = vac = wis = false;

	// The Multiplayer support for this dialog. You collect those parts together if more than one player.
	for(int i=0 ; i<m_NumPlayers ; i++)
	{
		joy |= m_Player[i].inventory.HasJoystick;
		bat |= m_Player[i].inventory.HasBattery;
		vac |= m_Player[i].inventory.HasVacuum;
		wis |= m_Player[i].inventory.HasWiskey;
	}

	// draw needed parts
	if (!joy) MessageBox->addTileAt(321,5<<3, 4<<3);
	if (!bat) MessageBox->addTileAt(322,14<<3, 4<<3);
	if (!vac) MessageBox->addTileAt(323,23<<3,4<<3);
	if (!wis) MessageBox->addTileAt(324,31<<3,4<<3);
	mMessageBoxes.push_back(MessageBox);
}

void CPlayGameVorticon::ShipEp3()
{
	// get one of four random strings and display it!!
	std::string strname = "EP3_SHIP"+ itoa((rand()%4)+1);
	mMessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString(strname)));
}

void CPlayGameVorticon::showKeensLeft()
{
	const unsigned int KEENSLEFT_X = 7;
	const unsigned int KEENSLEFT_Y = 10;

	if(mpKeenLeftSfc.empty())
	{
		int x,y,i,p;
		int boxY, boxH;
		CFont &Font = g_pGfxEngine->getFont(1);

		const unsigned int KEENSLEFT_W = 24;
		const unsigned int KEENSLEFT_H = 4;

		boxY = KEENSLEFT_Y - m_NumPlayers;
		boxH = KEENSLEFT_H + m_NumPlayers*2;

		SDL_Rect rect;
		rect.x = (KEENSLEFT_X+1)*8;	rect.y = (boxY+2)*8;
		rect.w = (KEENSLEFT_W+1)*8;	rect.h = (boxH)*8;
		SDL_Surface *boxsurface = SDL_CreateRGBSurface( SDL_SWSURFACE, rect.w, rect.h, RES_BPP, 0, 0, 0, 0 );

		rect.x = 8;	rect.y = 16;
		rect.w = (KEENSLEFT_W-1)*8;	rect.h = (boxH-3)*8;

		Uint8 r, g, b;
		Font.getBGColour(&r, &g, &b, true);
		Uint32 color = SDL_MapRGB( boxsurface->format, r, g, b);

		g_pGfxEngine->drawDialogBox( boxsurface, 0, 0, KEENSLEFT_W, boxH, color );
		SDL_FillRect(boxsurface, &rect, color );
		Font.getBGColour(&r, &g, &b, false);
		SDL_FillRect(boxsurface, &rect, SDL_MapRGB( boxsurface->format, r, g, b) );
		Font.drawFont( boxsurface, g_pBehaviorEngine->getString("LIVES_LEFT"), 36, 8, true);


		y = 20;
		for(p=0; p<m_NumPlayers ; p++)
		{
			x = 12;
			for(i=0;i<m_Player[p].inventory.lives&&i<=10;i++)
			{
				g_pGfxEngine->getSprite(m_Player[p].playerbaseframe+PMAPDOWNFRAME)._drawSprite(boxsurface, x, y );
				x+=16;
			}
			y += 16;
		}

		const SDL_Surface *blit = g_pVideoDriver->mp_VideoEngine->getBlitSurface();
		mpKeenLeftSfc = SDL_ConvertSurface( boxsurface, blit->format, blit->flags );
		SDL_FreeSurface(boxsurface);
	}
	else
	{
		keenleft_rect.x = (KEENSLEFT_X+1)*8;
		keenleft_rect.y = (KEENSLEFT_Y - m_NumPlayers + 2)*8;
		keenleft_rect.w = mpKeenLeftSfc->w;
		keenleft_rect.h = mpKeenLeftSfc->h;


		if( g_pTimer->HasTimeElapsed(3000) || g_pInput->getPressedAnyCommand() )
		{
			m_showKeensLeft = false;
			mpKeenLeftSfc.tryDeleteData();
		}

		g_pVideoDriver->mDrawTasks.add( new BlitSurfaceTask( mpKeenLeftSfc, NULL,  &keenleft_rect ) );

	}
}

int CPlayGameVorticon::getTeleporterInfo(int objectID)
{
	if(m_Episode == 1) {
		if( objectID > 33 && objectID < 47 ) return objectID;
	}
	else if(m_Episode == 3) {
		if( (objectID & 0xF00) == 0xF00) return objectID;
	}
	return 0;
}


/**
 * Teleporter part
 */
void CPlayGameVorticon::teleportPlayer(int objectID, CPlayer &player)
{
	int destx, desty;
	int origx, origy;
	mMap->findObject(objectID, &origx, &origy);
	CTeleporter *teleporter = new CTeleporter( mMap.get(), m_Player,origx<<CSF, origy<<CSF);
	teleporter->solid = false;
	teleporter->direction = TELEPORTING_IN;
	if(m_Episode == 1)
		readTeleportDestCoordinatesEP1(objectID, destx, desty);
	else if(m_Episode == 3)
		readTeleportDestCoordinatesEP3(objectID, destx, desty);
	teleporter->destx = destx>>TILE_S;
	teleporter->desty = desty>>TILE_S;
	teleporter->whichplayer = player.m_index;
	m_Object.push_back(teleporter);
}

void CPlayGameVorticon::readTeleportDestCoordinatesEP1(int objectID, int &destx, int &desty)
{
	destx = desty = 0;

	std::vector<stTeleporterTable>::iterator TTable =
			g_pBehaviorEngine->getTeleporterTable().begin();
	size_t i = 0;
	for( ; TTable != g_pBehaviorEngine->getTeleporterTable().end() ; TTable++, i++ )
	{
		if(TTable->objectnumber2 == objectID || TTable->objectnumber1 == objectID)
		{
			destx = TTable->x;
			desty = TTable->y;
			break;
		}
	}
}

void CPlayGameVorticon::readTeleportDestCoordinatesEP3(int objectID, int &destx, int &desty)
{
	destx = desty = 0;

	int newObject = objectID & 0x00F;
	newObject <<= 4;
	newObject += 0xF00; // Now its a teleporter, we only need to find the right one on the map

	for(int i=newObject; i<newObject+0x10 ; i++)
	{
		if(mMap->findObject(i, &destx, &desty))
		{
			destx <<= TILE_S;
			desty <<= TILE_S;
			return;
		}
	}
}
