/*
 * CLick.cpp
 *
 *  Created on: 04.08.2011
 *      Author: gerstrong
 */

#include "CLick.h"

#include "engine/galaxy/ai/CPlayerBase.h"
#include "misc.h"

namespace galaxy {

enum LICK_ACTIONS
{
A_LICK_HOP = 0,
A_LICK_LAND = 3,
A_LICK_BREATHE = 4,
A_LICK_STUNNED = 12
};

const int CSF_MIN_DISTANCE_TO_BREATHE = 2<<CSF;
const int CSF_DISTANCE_TO_FOLLOW_TOLERANCE = 2<<CSF;

const int LICK_HOP_X_SPEED = 50;
const int LICK_HOP_Y_SPEED = 50;

const int LICK_BREATHE_TIMER = 100;
const int LICK_HOP_TIME = 20;
const int LICK_LAND_TIME = 20;

CLick::CLick(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y) :
CStunnable(pmap, foeID, x, y),
m_timer(0)
{
	mActionMap[A_LICK_HOP] = (void (CStunnable::*)()) (&CLick::processHop);
	mActionMap[A_LICK_LAND] = (void (CStunnable::*)()) (&CLick::processLand);
	mActionMap[A_LICK_BREATHE] = (void (CStunnable::*)()) (&CLick::processBreathe);
	mActionMap[A_LICK_STUNNED] = &CStunnable::processGettingStunned;

	setupGalaxyObjectOnMap(0x2FC6, A_LICK_HOP);

	xDirection = LEFT;
}

void CLick::process()
{
	performCollisions();
	performGravityMid();

	if( blockedl )
		xDirection = RIGHT;
	else if( blockedr )
		xDirection = LEFT;

	(this->*mp_processState)();

	if(!processActionRoutine())
			exists = false;
}

void CLick::getTouchedBy(CSpriteObject &theObject)
{
	if(dead || theObject.dead)
		return;

	CStunnable::getTouchedBy(theObject);

	// Was it a bullet? Than make it stunned.
	if( dynamic_cast<CBullet*>(&theObject) )
	{
		setAction( A_LICK_STUNNED );
		theObject.dead = true;
		dead = true;
	}

	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
		if(getActionNumber(A_LICK_BREATHE))
		{
			player->kill();
		}
	}
}


bool CLick::isNearby(CSpriteObject &theObject)
{

	if( !getProbability(80) )
		return false;

	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
		const int dy = abs(player->getYMidPos() - getYMidPos());
		const int dx = player->getXMidPos() - getXMidPos();

		if( dy > CSF_MIN_DISTANCE_TO_BREATHE )
			return false;

		if( dx<-CSF_DISTANCE_TO_FOLLOW_TOLERANCE )
			xDirection = LEFT;
		else if( dx>+CSF_DISTANCE_TO_FOLLOW_TOLERANCE )
			xDirection = RIGHT;

		if(getActionNumber(A_LICK_LAND))
		{
			int absdx = (dx<0) ? -dx : dx;

			if( absdx < CSF_MIN_DISTANCE_TO_BREATHE )
			{
				setAction(A_LICK_BREATHE);
				playSound(SOUND_LICK_FIREBREATH);
				m_timer = LICK_BREATHE_TIMER;
			}
		}

	}

	return true;
}

void CLick::processHop()
{
	// Move left or right according to set direction
	if(xDirection == RIGHT)
		moveRight(LICK_HOP_X_SPEED);
	else if(xDirection == LEFT)
		moveLeft(LICK_HOP_X_SPEED);

	m_timer--;
	if( m_timer <= 0 )
	{
		setAction( A_LICK_LAND );
		m_timer = LICK_LAND_TIME;
	}
}

void CLick::processLand()
{
	// After a moment he might hop again
	m_timer--;
	if(blockedd && m_timer <= 0)
	{
		setAction( A_LICK_HOP );
		m_timer = LICK_HOP_TIME;
		yinertia = -LICK_HOP_Y_SPEED;
	}
}

void CLick::processBreathe()
{
	// Breathe for a brief moment
	m_timer--;
	if(getActionStatus(A_LICK_HOP+2) || m_timer <= 0)
	{
		m_timer = LICK_HOP_TIME;
		setAction( A_LICK_HOP );
		yinertia = -LICK_HOP_Y_SPEED;
	}
}


} /* namespace galaxy */
