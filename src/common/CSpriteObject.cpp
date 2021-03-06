/*
 * CSpriteObject.cpp
 *
 *  Created on: 17.05.2009
 *      Author: gerstrong
 */

#include "engine/spritedefines.h"
#include "CSpriteObject.h"
#include "CLogFile.h"
#include "sdl/CVideoDriver.h"

int CSpriteObject::m_number_of_objects = 0; // The current number of total objects we have within the game!

///
// Initialization Routine
///
CSpriteObject::CSpriteObject(CMap *pmap, Uint32 x, Uint32 y) :
mHealthPoints(1),
sprite(BLANKSPRITE),
mp_Map(pmap),
m_blinktime(0),
m_invincible(false),
m_Pos(x,y),
transluceny(0)
{
	falling = false;
	m_number_of_objects++;
	exists = true;
	solid = true;
	inhibitfall = false;
	canbezapped = false;
	onscreen = false;
	yinertia = 0;
	xinertia = 0;
	onslope = false;
	scrx = scry = 0;
	dead = false;
	hasbeenonscreen = false;
	honorPriority = true;
	dontdraw = false;
	cansupportplayer = false;
	dying = false;
	yDirection = xDirection = 0;
	supportedbyobject = false;

	blockedd = false;
	blockedu = false;
	blockedl = false;
	blockedr = false;
}


void CSpriteObject::setScrPos( int px, int py )
{
	scrx = px;
	scry = py;
}

// This functions checks, if the enemy is near to the player. In case, that it is
// it will return true. Other case it will return false.
// This used for objects that only can trigger, when it's really worth to do so.
bool CSpriteObject::calcVisibility()
{
	int visibility = g_pBehaviorEngine->getPhysicsSettings().misc.visibility;

	SDL_Rect gameres = g_pVideoDriver->getGameResolution().SDLRect();

	const Uint32 left = (((mp_Map->m_scrollx<<STC)-(visibility<<CSF))<0) ? 0 :
							(mp_Map->m_scrollx<<STC)-(visibility<<CSF);
	const Uint32 right = ((mp_Map->m_scrollx+gameres.w)<<STC)+(visibility<<CSF);
	const Uint32 up = (((mp_Map->m_scrolly<<STC)-(visibility<<CSF))<0) ? 0 :
							(mp_Map->m_scrolly<<STC)-(visibility<<CSF);
	const Uint32 down = ((mp_Map->m_scrolly+gameres.h)<<STC)+(visibility<<CSF);

	return ( right > m_Pos.x && left < m_Pos.x && down > m_Pos.y && up < m_Pos.y );
}




bool CSpriteObject::verifyForFalling()
{
	if( !blockedd )
	{
		// This will check three points and avoid that keen falls on sloped tiles
		const int &fall1 = mp_Map->getPlaneDataAt(1, getXMidPos(), getYDownPos());
		const int &fall2 = mp_Map->getPlaneDataAt(1, getXMidPos(), getYDownPos()+(1<<(CSF)));
		const int &fall3 = mp_Map->getPlaneDataAt(1, getXMidPos(), getYDownPos()+(2<<(CSF)));
		const CTileProperties &TileProp1 = g_pBehaviorEngine->getTileProperties(1)[fall1];
		const CTileProperties &TileProp2 = g_pBehaviorEngine->getTileProperties(1)[fall2];
		const CTileProperties &TileProp3 = g_pBehaviorEngine->getTileProperties(1)[fall3];
		const bool nothing_on_feet = (TileProp1.bup == 0);
		const bool nothing_below_feet = (TileProp2.bup == 0) && (TileProp3.bup == 0);
		const bool can_fall = (nothing_on_feet && nothing_below_feet);

		if(can_fall)
		{
			return true;
		}
		else
		{
			// Force the player a bit down, so he will never fall from sloped tiles
			moveDown(100);
		}
	}

	return false;
}



void CSpriteObject::performCliffStop(const int &speed)
{
	if(verifyForFalling())
	{
		if( xDirection == RIGHT )
		{
			xDirection = LEFT;
			moveLeft( speed );
		}
		else
		{
			xDirection = RIGHT;
			moveRight( speed );
		}
	}
}






// Used in some setup mode, like putting the player to
// the current map position
void CSpriteObject::moveToForce(const VectorD2<int> &dir)
{
	m_Pos = dir;
}

void CSpriteObject::moveToForce(const int& new_x, const int& new_y)
{
	moveToForce(VectorD2<int>(new_x, new_y));
}

// For the vector functions
void CSpriteObject::moveDir(const VectorD2<int> &dir)
{
	moveXDir(dir.x);
	moveYDir(dir.y);
}

void CSpriteObject::moveToHorizontal(const int& new_x)
{
	const int pos_x = new_x - m_Pos.x;
	moveXDir(pos_x);
}

void CSpriteObject::moveToVertical(const int& new_y)
{
	const int pos_y = new_y - m_Pos.y;
	moveYDir(pos_y);
}

void CSpriteObject::moveTo(const VectorD2<Uint32> &new_loc)
{
	VectorD2<int> amount = new_loc - m_Pos;

	moveXDir(amount.x);
	moveYDir(amount.y);
}

void CSpriteObject::moveTo(const int new_x, const int new_y)
{
	moveTo(VectorD2<Uint32>(new_x, new_y));
}

void CSpriteObject::moveXDir(const int amount, const bool force)
{
	if(amount<0)
		moveLeft(-amount, force);
	else if(amount>0)
		moveRight(amount, force);
}
void CSpriteObject::moveYDir(const int amount)
{
	if(amount<0)
		moveUp(-amount);
	else if(amount>0)
		moveDown(amount);
}

void CSpriteObject::moveLeft(const int amnt, const bool force)
{
	if(amnt <= 0)
		return;

	m_EventCont.add(new ObjMove(-amnt,0));
}

void CSpriteObject::moveRight(const int amnt, const bool force)
{
	if(amnt <= 0)
		return;

	m_EventCont.add(new ObjMove(amnt,0));
}

void CSpriteObject::moveUp(const int amnt)
{
	if(amnt <= 0)
		return;

	m_EventCont.add(new ObjMove(0,-amnt));
}

void CSpriteObject::moveDown(const int amnt)
{
	// If zero was given as argument return.
	if(amnt <= 0)
		return;

	m_EventCont.add(new ObjMove(0, amnt));
}

// This decreases the inertia we have of the object in X-direction.
// It should be used for objects, where it must be assured, that the inertia can get
// zero and not pass that limit
void CSpriteObject::decreaseXInertia(const int value)
{
	if(xinertia < 0)
	{
		if(xinertia+value > 0) xinertia = 0;
		else xinertia += value;
	}
	else if(xinertia > 0)
	{
		if(xinertia-value < 0) xinertia = 0;
		else xinertia -= value;
	}
}

// handles inertia and friction for the X direction
void CSpriteObject::InertiaAndFriction_X(const int friction_rate)
{
	int dx=xinertia;
	// check first if the player is not blocked
	if( (!blockedr and dx>0) or (!blockedl and dx<0) )
		moveXDir(dx);
	else
		xinertia = 0;

	// and apply friction to xinertia
	// when pogoing apply friction till we get down to PFASTINCMAXSPEED
	// then stop the friction
	decreaseXInertia(friction_rate);
}

VectorD2<Uint32> CSpriteObject::getPosition() const
{ return m_Pos; }
Uint32 CSpriteObject::getXPosition() const
{ return m_Pos.x; }
Uint32 CSpriteObject::getYPosition() const
{ return m_Pos.y; }
Uint32 CSpriteObject::getXLeftPos()
{ return m_Pos.x+m_BBox.x1; }
Uint32 CSpriteObject::getXRightPos()
{ return m_Pos.x+m_BBox.x2; }
Uint32 CSpriteObject::getXMidPos()
{ return m_Pos.x+(m_BBox.x2-m_BBox.x1)/2; }
Uint32 CSpriteObject::getYUpPos()
{ return m_Pos.y+m_BBox.y1; }
Uint32 CSpriteObject::getYDownPos()
{ return m_Pos.y+m_BBox.y2; }
Uint32 CSpriteObject::getYMidPos()
{ return m_Pos.y+(m_BBox.y2-m_BBox.y1)/2; }


void CSpriteObject::processFallPhysics(const int boost)
{
	CPhysicsSettings &Physics = g_pBehaviorEngine->getPhysicsSettings();

	// In this case foe is jumping?
	// Not sure here. We should use another variable...
	if(yinertia<0 && !blockedu)
	{
		moveUp(-yinertia);
		yinertia += boost;
	}
	else if( yinertia>=0 && !blockedd )
	{
		moveDown(yinertia);

		// gradually increase the fall speed up to maximum rate
		if (yinertia>Physics.max_fallspeed)
			yinertia = Physics.max_fallspeed;
		else if (yinertia<Physics.max_fallspeed)
			yinertia += boost;
	}

	// hit floor or ceiling? set inertia to zero
	if( (blockedd && yinertia>0) || (blockedu && yinertia<0) )
		yinertia = 0;

	// If object is not falling (yinertia >= 0) and blocked he cannot be falling
	if(blockedd && yinertia>=0)
		falling = false;
}

void CSpriteObject::processFallPhysics()
{
	CPhysicsSettings &Physics = g_pBehaviorEngine->getPhysicsSettings();
	processFallPhysics(Physics.fallspeed_increase);
}

/**
 * processes falling of an object. Can be player or any other foe
 */
void CSpriteObject::processFalling()
{
	// CAUTION: There is a difference between falling and going down with the gravity...

	// So it reaches the maximum of fallspeed
	if(!inhibitfall)
	{
		processFallPhysics();
	}
	else
	{
		moveYDir(yinertia);
	}

	// sometimes, due to mistakes on the map, some foe are embedded into blocks!
	// In order to avoid, that they can't get out, pull them out of there!
}

void CSpriteObject::getShotByRay(object_t &obj_type)
{
	if( !m_invincible && mHealthPoints>0)
	{
		if(mHealthPoints>1 && g_pVideoDriver->getSpecialFXConfig())
			blink(10);
		mHealthPoints--;
	}
}

// anything (players/enemies) occupying the map tile at [mpx,mpy] is killed
void CSpriteObject::kill_intersecting_tile(int mpx, int mpy, CSpriteObject &theObject)
{
	 unsigned int xpix,ypix;
	 unsigned int x, y;
	 xpix = mpx<<CSF;
	 ypix = mpy<<CSF;

	 x = theObject.getXMidPos();
	 y = theObject.getYUpPos();
	 if (theObject.exists)
	 {
		 if (xpix-(1<<CSF) <= x && xpix+(1<<CSF) >= x)
		 {
			 if (ypix <= y && ypix+(1<<CSF) >= y)
			 {
				 theObject.kill();
				 theObject.dontdraw = true;
			 }
		 }
	 }
}



// Just kills the object
void CSpriteObject::kill()
{
	if ( exists && canbezapped )
	{
		mHealthPoints = 0;
		dead = true;
	}
}

/**
 * \brief This function triggers the blinking behavior of an object.
 * 		  Normally this happens, when an enemy gets shot and can get
 * 		  multiple hits.
 * \param frametime	amount of drawn blinking frames during the blitting.
 * 					Every draw cycle performs one
 */
void CSpriteObject::blink(Uint16 frametime)
{	m_blinktime = frametime; }

void CSpriteObject::playSound( const GameSound snd,
			    const SoundPlayMode mode )
{
	g_pSound->playStereofromCoord(snd, mode, scrx);
}


////
// For drawing
////

// Functions finally draws the object also considering that there could be a masked
// or priority tile!
void CSpriteObject::draw()
{
	if( sprite == BLANKSPRITE || dontdraw )
		return;

	CSprite &Sprite = g_pGfxEngine->getSprite(sprite);

	scrx = (m_Pos.x>>STC)-mp_Map->m_scrollx;
	scry = (m_Pos.y>>STC)-mp_Map->m_scrolly;

	SDL_Rect gameres = g_pVideoDriver->getGameResolution().SDLRect();

	if( scrx < gameres.w && scry < gameres.h && exists )
	{
		Uint16 showX = scrx+Sprite.getXOffset();
		Uint16 showY = scry+Sprite.getYOffset();
		if(m_blinktime > 0)
		{
			Sprite.drawBlinkingSprite( showX, showY );
			m_blinktime--;
		}
		else
		{
			Sprite.drawSprite( showX, showY, (255-transluceny) );
		}
		hasbeenonscreen = true;
	}
}

///
// Cleanup Routine
///
CSpriteObject::~CSpriteObject()
{
	if(m_number_of_objects > 0)
		m_number_of_objects--;
}

