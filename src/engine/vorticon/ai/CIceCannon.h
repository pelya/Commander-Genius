/*
 * icecannon.h
 *
 *  Created on: 06.08.2009
 *      Author: gerstrong
 */

#ifndef ICECANNON_H_
#define ICECANNON_H_

#include "engine/vorticon/CVorticonSpriteObject.h"
#include "../../../common/CPlayer.h"
#include <vector>

unsigned int rnd(void);

class CIceCannon : public CVorticonSpriteObject
{
public:
	CIceCannon(CMap *p_map, Uint32 x, Uint32 y,
			std::vector<CPlayer>& Player, std::vector<CVorticonSpriteObject*>& Object,
			int vector_x, int vector_y );
	void process();

private:
	int vector_x, vector_y;

	std::vector<CPlayer>& m_Player;
	std::vector<CVorticonSpriteObject*>& m_Object;
};

class CIceChunk : public CVorticonSpriteObject
{
public:
	CIceChunk(CMap *p_map, Uint32 x, Uint32 y, Uint32 vx, Uint32 vy,
			std::vector<CPlayer>& Player, std::vector<CVorticonSpriteObject*>& Object);
	void process();
	void smash();
private:
	int vector_x, vector_y;
	int veloc_x, veloc_y;
	std::vector<CPlayer>& m_Player;
	std::vector<CVorticonSpriteObject*>& m_Object;
};

class CIceBit : public CVorticonSpriteObject
{
public:
	CIceBit(CMap *p_map, Uint32 x, Uint32 y, Uint32 vec_x, Uint32 vec_y);
	void process();

	int vector_x, vector_y;
	int veloc_x, veloc_y;
	int timer;
};

#endif /* ICECANNON_H_ */
