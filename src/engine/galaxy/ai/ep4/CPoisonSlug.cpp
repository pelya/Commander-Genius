/*
 * CPoisonSlug.cpp
 *
 *  Created on: 23FEB2011
 *      Author: FCTW
 */


#include "CPoisonSlug.h"
#include "CSlugSlime.h"
#include "engine/galaxy/ai/CPlayerBase.h"
#include "misc.h"

namespace galaxy {


enum SLUGACTIONS
{
A_SLUG_MOVE = 0,	/* Ordinary slug_move action */
A_SLUG_POOING = 2,
A_SLUG_STUNNED = 3,
A_SLUG_STUNNED_ALT = 4
};

const int SLUG_MOVE_SPEED = 1;
const int SLUG_MOVE_TIMER = 10;

CPoisonSlug::CPoisonSlug(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y,
		std::vector< SmartPointer<CGalaxySpriteObject> >&ObjectPtrs) :
CStunnable(pmap, foeID, x, y),
m_ObjectPtrs(ObjectPtrs),
m_timer(0)
{
	mActionMap[A_SLUG_MOVE] = (void (CStunnable::*)()) &CPoisonSlug::processCrawling;
	mActionMap[A_SLUG_POOING] = (void (CStunnable::*)()) &CPoisonSlug::processPooing;
	mActionMap[A_SLUG_STUNNED] = &CStunnable::processGettingStunned;
	mActionMap[A_SLUG_STUNNED_ALT] = &CStunnable::processGettingStunned;

	setupGalaxyObjectOnMap(0x2012, A_SLUG_MOVE);

	xDirection = LEFT;

	CSprite &rSprite = g_pGfxEngine->getSprite(sprite);
	performCollisions();
	processMove( 0, rSprite.m_bboxY1-rSprite.m_bboxY2 );
	if(!processActionRoutine())
			exists = false;
}





void CPoisonSlug::processCrawling()
{
	// Check if there is a cliff
	//performCliffStop(m_Action.velX<<1);


	if( m_timer < SLUG_MOVE_TIMER )
	{
		m_timer++;
		return;
	}
	else
	{
		m_timer = 0;
	}

	// Chance to poo
	if( getProbability(30) )
	{
		m_timer = 0;
		setAction( A_SLUG_POOING );
		playSound( SOUND_SLUG_DEFECATE );
		m_ObjectPtrs.push_back(new CSlugSlime(mp_Map, 0, getXLeftPos(), getYDownPos()-(8<<STC)));
		return;
	}

	// Move normally in the direction
	if( xDirection == RIGHT )
	{
		moveRight( m_Action.velX<<1 );
	}
	else
	{
		moveLeft( m_Action.velX<<1 );
	}

}





void CPoisonSlug::processPooing()
{
	if( getActionStatus(A_SLUG_MOVE) )
		setAction(A_SLUG_MOVE);
}


void CPoisonSlug::getTouchedBy(CSpriteObject &theObject)
{
	if(dead || theObject.dead)
		return;

	CStunnable::getTouchedBy(theObject);

	// Was it a bullet? Than make it stunned.
	if( dynamic_cast<CBullet*>(&theObject) )
	{
		setAction( rand()%2 ? A_SLUG_STUNNED : A_SLUG_STUNNED_ALT );
		dead = true;
		theObject.dead = true;
	}

	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
		player->kill();
	}
}


int CPoisonSlug::checkSolidD( int x1, int x2, int y2, const bool push_mode )
{
	turnAroundOnCliff( x1, x2, y2 );

	return CGalaxySpriteObject::checkSolidD(x1, x2, y2, push_mode);
}


void CPoisonSlug::process()
{
	performCollisions();

	if(!blockedd)
		performGravityMid();

	(this->*mp_processState)();

	if( blockedl )
		xDirection = RIGHT;
	else if(blockedr)
		xDirection = LEFT;

	if(!processActionRoutine())
		exists = false;
}

}
