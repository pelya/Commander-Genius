/*
 * CVorticonMapLoader.cpp
 *
 *  Created on: 09.10.2009
 *      Author: gerstrong
 */

#include "CVorticonMapLoader.h"
#include <iostream>
#include <fstream>
#include "FindFile.h"
#include "CLogFile.h"
#include "fileio.h"
#include "fileio/ResourceMgmt.h"
#include "fileio/compression/CRLE.h"
#include "common/CBehaviorEngine.h"
#include "graphics/CGfxEngine.h"
#include "sdl/CVideoDriver.h"
#include "CResourceLoader.h"

#include "engine/vorticon/ai/CYorp.h"
#include "engine/vorticon/ai/CGarg.h"
#include "engine/vorticon/ai/CAutoRay.h"
#include "engine/vorticon/ai/CVorticon.h"
#include "engine/vorticon/ai/CSectorEffector.h"
#include "engine/vorticon/ai/CVortiKid.h"
#include "engine/vorticon/ai/CVorticonElite.h"
#include "engine/vorticon/ai/CVortiMom.h"
#include "engine/vorticon/ai/CVortiNinja.h"
#include "engine/vorticon/ai/CBallJack.h"
#include "engine/vorticon/ai/CButler.h"
#include "engine/vorticon/ai/CDoor.h"
#include "engine/vorticon/ai/CFoob.h"
#include "engine/vorticon/ai/CTank.h"
#include "engine/vorticon/ai/CGuardRobot.h"
#include "engine/vorticon/ai/CTeleporter.h"
#include "engine/vorticon/ai/CMessie.h"
#include "engine/vorticon/ai/CMeep.h"
#include "engine/vorticon/ai/CPlatform.h"
#include "engine/vorticon/ai/CRope.h"
#include "engine/vorticon/ai/CScrub.h"
#include "engine/vorticon/ai/CIceCannon.h"
#include "engine/vorticon/ai/CSpark.h"

CVorticonMapLoader::CVorticonMapLoader( SmartPointer<CMap> &map,
						std::vector<CPlayer> *p_PlayerVect ) :
mpMap(map),
mp_vec_Player(p_PlayerVect)
{
	mp_objvect = NULL;
	m_checkpointset = false;
	m_NessieAlreadySpawned = false;
}

// Loads the map into the memory
bool CVorticonMapLoader::load( Uint8 episode, Uint8 level, const std::string& path, bool loadNewMusic, bool stategame )
{
	std::string levelname = "level";
	if(level < 10) levelname += "0";
	levelname += itoa(level) + ".ck" + itoa(episode);

	mpMap->resetScrolls();
	mpMap->m_gamepath = path;
	mpMap->m_worldmap = (level == 80);

	// HQ Music. Load Music for a level if you have HQP
	if(loadNewMusic)
	{
		g_pMusicPlayer->stop();

		// If no music from the songlist could be loaded try the normal table which
		// has another format. It is part of HQP
		if(!g_pMusicPlayer->LoadfromSonglist(path, level))
			g_pMusicPlayer->LoadfromMusicTable(path, levelname);
	}

	// decompress map RLEW data
	std::ifstream MapFile;
	bool fileopen = OpenGameFileR(MapFile, getResourceFilename(levelname,path,true,false), std::ios::binary);

	if (!fileopen)
	{
		// only record this error message on build platforms that log errors
		// to a file and not to the screen.
		g_pLogFile->ftextOut("MapLoader: unable to open file %s<br>", levelname.c_str());
		return 0;
	}
	g_pLogFile->ftextOut("MapLoader: file %s opened. Loading...<br>", levelname.c_str());

	MapFile.seekg (0, std::ios::beg);

	// load the compressed data into the memory
	std::vector<Uint8>	compdata;
	while( !MapFile.eof() )
	{
		compdata.push_back(static_cast<Uint8>(MapFile.get()));
	}

	MapFile.close();

	CRLE RLE;
	std::vector<Uint16> planeitems;
	RLE.expandSwapped(planeitems,compdata, 0xFEFE);

	// Here goes the memory allocation function
	mpMap->createEmptyDataPlane(1, planeitems.at(1), planeitems.at(2));

	int t;
	unsigned int planesize = 0;
	unsigned int curmapx=0, curmapy=0;
	planesize = planeitems.at(8);
	planesize /= 2; // Size of two planes, but we only need one


	const char &fixlevel_error = g_pBehaviorEngine->m_option[OPT_FIXLEVELERRORS].value;

	Uint32 c;
	for( c=17 ; c<planesize+17 ; c++ ) // Check against Tilesize
	{
		t = planeitems.at(c);

		if( fixlevel_error )
			fixLevelTiles(t, curmapx, curmapy, episode, level);

		addTile(t, curmapx, curmapy);

		curmapx++;
		if (curmapx >= mpMap->m_width)
		{
			curmapx = 0;
			curmapy++;
			if (curmapy >= mpMap->m_height) break;
		}

		if(t > 255)
			t=0; // If there are some invalid values in the file
	}

	// now do the sprites
	// get sprite data
	int resetcnt, resetpt;
	curmapx = curmapy = 0;
	resetcnt = resetpt = 0;

	if(mp_objvect && stategame == false)
	{
		std::vector<CVorticonSpriteObject*>::iterator obj = mp_objvect->begin();
		for( ; obj != mp_objvect->end() ; obj++ )
		{
			delete *obj;
			mp_objvect->pop_back();
		}

		mp_objvect->reserve(2000);

		for( c=planesize+17 ; c<2*planesize+16 ; c++ )
		{
			// in case the planesizes are bigger than the actual file content itself
			if(planeitems.size() <= c) break;

			t = planeitems.at(c);

			if (mpMap->m_worldmap) addWorldMapObject(t, curmapx, curmapy,  episode );
			else addEnemyObject(t, curmapx, curmapy, episode, level);

			curmapx++;
			if (curmapx >= mpMap->m_width)
			{
				curmapx = 0;
				curmapy++;
				if (curmapy >= mpMap->m_height) break;
			}

			if (++resetcnt==resetpt) curmapx = curmapy = 0;
		}

	}

	planeitems.clear();

	// Do some post calculations
	// Limit the scroll screens so the blocking (blue in EP1) tiles are3 never seen
	SDL_Rect gamerect = g_pVideoDriver->getGameResolution().SDLRect();
	mpMap->m_maxscrollx = (mpMap->m_width<<4) - gamerect.w - 32;
	mpMap->m_maxscrolly = (mpMap->m_height<<4) - gamerect.h - 32;

	// Set Map Delegation Object. This only gets the Pointer to the map instances x-y-scroll-buffers
	g_pVideoDriver->updateScrollBuffer( mpMap );

	return true;
}

void CVorticonMapLoader::addTile( Uint16 t, Uint16 x, Uint16 y )
{
	// Special cases. Those happen normally, when levels are replayed.
	// For example if one player has battery, the level won't show that item

	// Now set this this tile at pos(curmapx, curmapy)
	mpMap->setTile(x, y, t);
}

void CVorticonMapLoader::addWorldMapObject(unsigned int t, Uint16 x, Uint16 y, int episode)
{
	// This function add sprites on the map. Most of the objects are invisible.
	// TODO : Please convert this into ifs. There are more conditions than just switch.agree
	std::vector<CPlayer>::iterator it_player;
	switch(t)
	{
		case 0: break;       // blank
		case 255:            // player start
			if(!m_checkpointset)
			{
				it_player = mp_vec_Player->begin();
				for(; it_player != mp_vec_Player->end() ; it_player++ )
				{
					it_player->exists = false;
					it_player->moveToForce(x<<CSF, y<<CSF);
				}
			}
			mpMap->m_objectlayer[x][y] = 0;

			it_player = mp_vec_Player->begin();
			for(; it_player != mp_vec_Player->end() ; it_player++ )
			{
				it_player->setupforLevelPlay();
				it_player->solid = it_player->godmode;
			}

			break;
		case NESSIE_PATH:          // spawn nessie at first occurance of her path
			if (episode==3)
			{
				if (!m_NessieAlreadySpawned)
				{
					CMessie *messie = new CMessie(mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player);
					m_NessieAlreadySpawned = true;
					mp_objvect->push_back(messie);
				}
				mpMap->m_objectlayer[x][y] = NESSIE_PATH;
			}
			break;
		default:             // level marker

			// Taken from the original CloneKeen. If hard-mode chosen, swap levels 5 and 9 Episode 1
			if(episode == 1 && g_pBehaviorEngine->mDifficulty >= HARD)
			{
				if(t == 5)
					t = 9;
				else if(t == 9)
					t = 5;
			}

			mpMap->m_objectlayer[x][y] = t;

			if ((t&0x7fff) <= 16 && mp_vec_Player->front().mp_levels_completed[t&0x00ff])
			{
				// Change the level tile to a done sign
				int newtile = g_pBehaviorEngine->getTileProperties()[mpMap->at(x,y)].chgtile;

				// Consistency check! Some Mods have issues with that.
				if(episode == 1 || episode == 2)
				{
					//Use default small tile
					newtile = 77;

					// try to guess, if it is a 32x32 (4 16x16) Tile
					if(mpMap->at(x-1,y-1) == (unsigned int) newtile &&
							mpMap->at(x,y-1) == (unsigned int) newtile  &&
							mpMap->at(x-1,y) == (unsigned int) newtile)
					{
						mpMap->setTile(x-1, y-1, 78);
						mpMap->setTile(x, y-1, 79);
						mpMap->setTile(x-1, y, 80);
						newtile = 81; // br. this one
					}
				}
				else if(episode == 3)
				{
					newtile = 56;
					// try to guess, if it is a 32x32 (4 16x16) Tile
					if(mpMap->at(x-1, y-1) == (unsigned int) newtile &&
							mpMap->at(x, y-1) == (unsigned int) newtile  &&
							mpMap->at(x-1, y) == (unsigned int) newtile)
					{
						mpMap->setTile(x-1, y-1, 52);
						mpMap->setTile(x, y-1, 53);
						mpMap->setTile(x-1, y, 54);
						newtile = 55;
					}
				}
				mpMap->setTile(x, y, newtile);
			}
			break;
	}
}

void CVorticonMapLoader::addEnemyObject(unsigned int t, Uint16 x, Uint16 y, int episode, int level)
{
	mpMap->m_objectlayer[x][y] = t;

	if (t)
	{
		if (t==255) // The player in the level!
		{
			if(x >= mpMap->m_width-2) // Edge bug. Keen would fall in some levels without this.
				x = 4;

			if(y >= mpMap->m_height-2) // Edge bug. Keen would fall in some levels without this.
				y = 4;

			std::vector<CPlayer>::iterator it_player = mp_vec_Player->begin();
			for(; it_player != mp_vec_Player->end() ; it_player++ )
			{
				it_player->exists = false;
				it_player->moveToForce(x<<CSF, (y<<CSF)+275);
				it_player->setupforLevelPlay();
			}
		}
		else
		{
			CVorticonSpriteObject *enemyobject = NULL;

			switch(t)
			{
			case 0: break;
			case -1: break;
			case 1:  // yorp (ep1) vort (ep2&3)
				if (episode == 1)
				{
					enemyobject = new CYorp( mpMap.get(), *mp_vec_Player, x<<CSF, y<<CSF );
				}
				else if(episode == 2 || episode == 3)
				{
					enemyobject = new CVorticon( mpMap.get(), *mp_vec_Player, x<<CSF, y<<CSF);
				}
				break;
			case 2:    // garg (ep1) baby vorticon (ep2&3)
				if (episode == 1)
				{
					enemyobject = new CGarg( mpMap.get(), *mp_vec_Player, x<<CSF, y<<CSF );
				}
				else
				{
					enemyobject = new CVortikid( mpMap.get(), *mp_vec_Player, x<<CSF, y<<CSF );
				}

				break;
			case 3:    // vorticon (ep1) Vorticon Commander (ep2)
				if (episode==1)
				{
					CPhysicsSettings &Phy = g_pBehaviorEngine->getPhysicsSettings();

					size_t health = (level==16) ? Phy.vorticon.commander_hp : Phy.vorticon.default_hp;
					enemyobject = new CVorticon( mpMap.get(), *mp_vec_Player, x<<CSF, y<<CSF, health );
				}
				else if (episode==2)
				{
					enemyobject = new CVorticonElite( mpMap.get(), *mp_vec_Player,
							*mp_objvect, x<<CSF, y<<CSF);
				}
				else if (episode==3)
				{
					enemyobject = new CVortiMom( mpMap.get(), x<<CSF, y<<CSF,
											*mp_vec_Player, *mp_objvect);
				}
				break;
			case 4:    // butler (ep1) or scrub (ep2) or meep (ep3)
				if (episode==1)
					enemyobject = new CButler( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player);
				else if (episode==2)
					enemyobject = new CScrub( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player);
				else if (episode==3)
					enemyobject = new CMeep( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player, *mp_objvect);
				break;
			case 5:    // tank robot (ep1&2) vorticon ninja (ep3)
				if (episode==1)
					enemyobject = new CTank( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player, *mp_objvect);
				else if (episode==2)
					enemyobject = new CGuardRobot( mpMap.get(), x<<CSF, y<<CSF, *mp_objvect);
				else if (episode==3)
					enemyobject = new CVortiNinja( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player);
				break;
			case 6:    // up-right-flying ice chunk (ep1) horiz platform (ep2)
				// foob (ep3)
				if (episode==1)
				{
					enemyobject = new CIceCannon( mpMap.get(), x<<CSF, y<<CSF,
							*mp_vec_Player,	*mp_objvect, 1, -1 );
				}
				else if (episode==2)
				{
					enemyobject = new CPlatform( mpMap.get(),x<<CSF, (y<<CSF)-(4<<STC), *mp_vec_Player);
				}
				else if (episode==3)
				{
					enemyobject = new CFoob( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player);
				}
				break;
			case 7:   // spark (ep2) ball (ep3) ice cannon upwards (ep1)
				if (episode==1)
				{
					enemyobject = new CIceCannon( mpMap.get(),x<<CSF, y<<CSF, *mp_vec_Player,*mp_objvect,0,-1);
				}
				else if (episode==2)
				{
					enemyobject = new CSpark( mpMap.get(), x<<CSF, y<<CSF, *mp_objvect);
				}
				else if (episode==3)
				{
					enemyobject = new CBallJack( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player, OBJ_BALL);
				}
				break;
			case 8:    // jack (ep3) and ice cannon down (ep1)
				if (episode==1)
				{
					enemyobject = new CIceCannon( mpMap.get(), x<<CSF, y<<CSF,*mp_vec_Player,*mp_objvect,0,1);
				}
				else if (episode==3)
				{
					enemyobject = new CBallJack( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player, OBJ_JACK);
				}
				break;
			case 9:    // up-left-flying ice chunk (ep1) horiz platform (ep3)
				if (episode==1)
				{
					enemyobject = new CIceCannon( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player,*mp_objvect,-1,-1);
				}
				else if (episode==3)
				{
					enemyobject = new CPlatform( mpMap.get(), x<<CSF, (y<<CSF)-(4<<STC), *mp_vec_Player);
				}
				break;
			case 10:   // rope holding the stone above the final vorticon (ep1)
				// vert platform (ep3)
				if (episode==1)
				{
					enemyobject = new CRope( mpMap.get(), x<<CSF, y<<CSF);
				}
				else if (episode==3)
				{
					enemyobject = new CPlatformVert( mpMap.get(), x<<CSF, y<<CSF, *mp_vec_Player);
				}
				break;
			case 11:   // jumping vorticon (ep3)
				if (episode==3)
				{
					enemyobject = new CVorticon( mpMap.get(), *mp_vec_Player, x<<CSF, y<<CSF );
				}
				break;
			case 12:   // sparks in mortimer's machine
				enemyobject = new CSectorEffector( mpMap.get(), x<<CSF, y<<CSF,
						*mp_vec_Player,*mp_objvect, SE_MORTIMER_SPARK);
				enemyobject->solid = false;
				break;
			case 13:   // mortimer's heart
				enemyobject = new CSectorEffector( mpMap.get(), x<<CSF, y<<CSF,
						*mp_vec_Player,*mp_objvect, SE_MORTIMER_HEART);
				enemyobject->solid = false;
				break;
			case 14:   // right-pointing raygun (ep3)
				enemyobject = new CAutoRay(mpMap.get(), x<<CSF, y<<CSF, *mp_objvect, CAutoRay::HORIZONTAL);
				break;
			case 15:   // vertical raygun (ep3)
				enemyobject = new CAutoRay(mpMap.get(), x<<CSF, y<<CSF, *mp_objvect, CAutoRay::VERTICAL);
				break;
			case 16:  // mortimer's arms
				enemyobject = new CSectorEffector( mpMap.get(), x<<CSF, y<<CSF,
						*mp_vec_Player,*mp_objvect, SE_MORTIMER_ARM );
				enemyobject->solid = false;
				break;
			case 17:  // mortimer's left leg
				enemyobject = new CSectorEffector( mpMap.get(), x<<CSF, y<<CSF,
						*mp_vec_Player,*mp_objvect, SE_MORTIMER_LEG_LEFT );
				enemyobject->solid = false;
				break;
			case 18:  // mortimer's right leg
				enemyobject = new CSectorEffector( mpMap.get(), x<<CSF, y<<CSF,
						*mp_vec_Player,*mp_objvect, SE_MORTIMER_LEG_RIGHT );
				enemyobject->solid = false;
				break;
			default:
				g_pLogFile->ftextOut(PURPLE,"unknown enemy type %d at (%d,%d)<br>", t, x, y);
				break;
			}


			if(enemyobject != NULL)
				mp_objvect->push_back(enemyobject);
		}
	}
}

/**
 * \brief As the original Commander Keen games have bugs, this function will fix them. it only will trigger.
 * \param currentTile	Number of the tile that will be modified to get the bug fixed.
 * \param curmapX		X-Coordinate of the map not CSFed
 * \param curmapY		Y-Coordinate of the map not CSFed
 * \param episode		Episode of the game
 */
void CVorticonMapLoader::fixLevelTiles(int &currentTile, const Uint16 curmapX, const Uint16 curmapY, const int episode, const int level)
{
	if( episode == 1 && level == 14 )
	{
		if( (curmapX == 14 && curmapY == 10) || (curmapX == 13 && curmapY == 13) )
			currentTile = 143;
		else if( (curmapX == 14 && curmapY == 11) || (curmapX == 13 && curmapY == 14) )
			currentTile = 331;
	}
	else if( episode == 2 && level == 15 )
	{
		if( curmapX == 50 && curmapY == 18 )
			currentTile = 187;
	}
	else if( episode == 3 && level == 8 )
	{
		if( ( curmapX == 77 && curmapY == 52 ) ||
				( ( curmapX == 94 || curmapX == 95 || curmapX == 96 ) && curmapY == 15 ) )
			currentTile = 169;
	}
	else if( episode == 3 && level == 15 )
	{
		if( ( curmapX == 32 || curmapX == 33 ) && curmapY == 113 )
			currentTile = 482;
	}
}
