/*
 * CVorticonMapLoader.h
 *
 *  Created on: 09.10.2009
 *      Author: gerstrong
 */

#ifndef CVorticonMapLoader_H_
#define CVorticonMapLoader_H_

#include "CMap.h"
#include "CPlayer.h"
#include "CSpriteObject.h"
#include "options.h"
#include "sdl/music/CMusic.h"
#include "common/CBehaviorEngine.h"
#include "SmartPointer.h"
#include <string>
#include <vector>

class CVorticonMapLoader
{
public:
	CVorticonMapLoader(SmartPointer<CMap> &map,
				std::vector<CPlayer> *p_PlayerVect = NULL);
	
	bool load( Uint8 episode, Uint8 level, const std::string& path, bool loadNewMusic=true, bool stategame=false );
	
	void addTile( Uint16 t, Uint16 x, Uint16 y );
	void addWorldMapObject(unsigned int t, Uint16 x, Uint16 y, int episode);
	void addEnemyObject(unsigned int t, Uint16 x, Uint16 y, int episode, int level);
	void fixLevelTiles(int &currentTile, const Uint16 curmapX, const Uint16 curmapY, const int episode, const int level);
	
	bool m_checkpointset;
	bool m_NessieAlreadySpawned;
	std::vector<CVorticonSpriteObject*> *mp_objvect;

private:
	SmartPointer<CMap>& mpMap;
	std::vector<CPlayer> *mp_vec_Player;
};

#endif /* CVorticonMapLoader_H_ */
