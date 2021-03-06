/*
 * CStunnable.cpp
 *
 *  Created on: 23.06.2011
 *      Author: gerstrong
 *
 *  Those are in the galaxy Engine the stunnable objects
 *  Most of the enemies but not all foes can inherit this object
 *  and will get the stars when stunned or not, depending on how stunnable
 *  the enemies are...
 */

#include "CStunnable.h"
#include "CBullet.h"
#include "common/CBehaviorEngine.h"
#include "engine/galaxy/ai/CStarRing.h"


namespace galaxy
{

CStunnable::CStunnable(	CMap *pmap,
						const Uint16 foeID,
						Uint32 x,
						Uint32 y ) :
CGalaxySpriteObject( pmap, foeID, x, y ),
m_stunned(false),
mp_processState(NULL)
{
	m_invincible = false;
}

void CStunnable::getTouchedBy(CSpriteObject &theObject)
{
	CBullet *bullet = dynamic_cast<CBullet*>(&theObject);
	if( theObject.exists && bullet != NULL )
	{
		bullet->setAction(A_KEENSHOT_IMPACT);
		bullet->playSound( SOUND_SHOT_HIT );
	}
}

void CStunnable::processGettingStunned()
{
	if(blockedd)
	{
		if( !m_invincible )
		{
			CSprite &StarRing = g_pGfxEngine->getSprite( STARRING_SPRITE );

			// Calculate the Position of the Star-Ring. Make it centered and above its head
			const Uint32 star_x = getXMidPos() - ( (StarRing.getWidth()<<STC)/2 );
			const Uint32 star_y = getYUpPos()  - ( StarRing.getHeight()<<STC );

			EventSpawnObject *Ev = new EventSpawnObject( new CStarRing(mp_Map, 0, star_x, star_y) );
			g_pBehaviorEngine->m_EventList.add( Ev );
			mp_processState = &CStunnable::processStunned;
		}
	}
}

void CStunnable::processStunned()
{ }


void CStunnable::setActionForce(const size_t ActionNumber)
{
	CGalaxySpriteObject::setActionForce(ActionNumber);

	if( mActionMap.find(ActionNumber) != mActionMap.end() )
		mp_processState = mActionMap[ActionNumber];
	else
		setActionForce(0); // This might happen, when the action-map is incomplete
}

};
