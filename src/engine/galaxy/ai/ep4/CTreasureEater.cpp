/*
 * CTreasureEater.cpp
 *
 *  Created on: 17.07.2012
 *      Author: gerstong
 */

#include "CTreasureEater.h"
#include "engine/galaxy/ai/CPlayerLevel.h"
#include "engine/galaxy/ai/CBullet.h"
#include "misc.h"

namespace galaxy {


const int A_SMIRKY_LOOK = 0;
const int A_SMIRKY_TELEPORT = 2;
const int A_SMIRKY_HOP = 10;
const int A_SMIRKY_STUNNED = 14;

const int HOP_INERTIA = 120;
const int SMIRLY_HOP_TIMER = 10;


CTreasureEater::CTreasureEater(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y) :
CStunnable(pmap, foeID, x, y),
mTimer(0),
mTeleported(false),
mStolen(false)
{
	mActionMap[A_SMIRKY_LOOK]    = (void (CStunnable::*)()) &CTreasureEater::processLooking;
	mActionMap[A_SMIRKY_TELEPORT]    = (void (CStunnable::*)()) &CTreasureEater::processTeleporting;
	mActionMap[A_SMIRKY_HOP]     = (void (CStunnable::*)()) &CTreasureEater::processHopping;
	mActionMap[A_SMIRKY_STUNNED] = &CStunnable::processGettingStunned;

	setupGalaxyObjectOnMap( 0x31E2, A_SMIRKY_LOOK );
	xDirection = LEFT;

	CSprite &rSprite = g_pGfxEngine->getSprite(sprite);
	performCollisions();
	processMove( 0, rSprite.m_bboxY1-rSprite.m_bboxY2 );
	if(!processActionRoutine())
			exists = false;

}


bool CTreasureEater::isNearby(CSpriteObject &theObject)
{
	if( !getProbability(80) )
		return false;

	// TODO: identify one item and try to get it!
	/*if( getActionNumber(A_SMIRKY_LOOK) )
	{

	}*/



	return true;
}


void CTreasureEater::getTouchedBy(CSpriteObject &theObject)
{
	if(dead || theObject.dead )
		return;


	CStunnable::getTouchedBy(theObject);

	// Was it a bullet? Than make it stunned.
	if( dynamic_cast<CBullet*>(&theObject) )
	{
		setAction( A_SMIRKY_STUNNED );
		setActionSprite();
		dead = true;
		theObject.dead = true;
	}

	// Here go the items

}


void CTreasureEater::processLooking()
{
	if( mTimer < SMIRLY_HOP_TIMER )
	{
		mTimer++;
		return;
	}
	else
	{
		mTimer = 0;
	}

	if(mEndOfAction)
	{

		if( getProbability(800) )
		{
			setAction(A_SMIRKY_HOP);
			yinertia = -HOP_INERTIA;
			inhibitfall = false;
		}
		else
		{
			setActionForce(A_SMIRKY_LOOK);
			inhibitfall = true;
		}
	}
}

void CTreasureEater::processTeleporting()
{
	if( getActionStatus(A_SMIRKY_HOP) )
	{
		setAction(A_SMIRKY_HOP);
		mStolen = false;
	}
}


void CTreasureEater::processHopping()
{
	if(yinertia >= 0)
	{
		if(blockedd || onslope)
		{
			if(mStolen)
			{
				setAction( A_SMIRKY_TELEPORT );
				mTeleported = true;
			}
			else
			{
				setAction( A_SMIRKY_LOOK );
			}

			inhibitfall = true;
			return;
		}
	}
}



void CTreasureEater::checkForItem()
{
	int l_x = getXLeftPos();
	int l_y = getYUpPos();
	int l_w = getXRightPos() - getXLeftPos();
	int l_h = getYDownPos() - getYUpPos();

	// So far we have tile item support only!
	// TODO: Need Sprite Item support and also this one should should be able to steal gems
	for( Uint32 i=21 ; i<=27 ; i++ )
	{
		if(hitdetectWithTilePropertyRect(i, l_x, l_y, l_w, l_h, 1<<STC))
		{
			const int lc_x = l_x>>CSF;
			const int lc_y = l_y>>CSF;
			mp_Map->setTile( lc_x, lc_y, 0, true, 1 );
			mStolen = true;
		}
	}
}


void CTreasureEater::process()
{
	performCollisions();

	if(!inhibitfall)
		performGravityMid();

	checkForItem();

	if( blockedl )
		xDirection = RIGHT;
	else if( blockedr )
		xDirection = LEFT;

	(this->*mp_processState)();

	if( !getActionNumber(A_SMIRKY_STUNNED) )
	{
		processActionRoutine();
	}
	else // TODO: This a workaround, because the last known action doesn't seem to be triggered
	{
		sprite = 231;
	}
}



} /* namespace galaxy */
