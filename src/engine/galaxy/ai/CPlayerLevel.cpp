/*
 * CPlayerLevel.cpp
 *
 *  Created on: 06.08.2010
 *      Author: gerstrong
 */

#include "CPlayerLevel.h"
#include "CBullet.h"
#include "common/CBehaviorEngine.h"
#include "platform/CPlatform.h"
#include "sdl/input/CInput.h"
#include "sdl/sound/CSound.h"
#include "sdl/CTimer.h"
#include "CVec.h"
#include "CLogFile.h"

namespace galaxy {


const int MAX_JUMPHEIGHT = 10;
const int MIN_JUMPHEIGHT = 5;

const int MAX_POGOHEIGHT = 20;
const int MIN_POGOHEIGHT = 5;
const int POGO_SLOWDOWN = 4;

const int POGO_START_INERTIA_VERT = -100;
const int POGO_START_INERTIA_MAX_VERT = -168;
const int POGO_START_INERTIA_IMPOSSIBLE_VERT = -180;
const int POGO_INERTIA_HOR_MAX = 64;
const int POGO_INERTIA_HOR_REACTION = 2;

const int FIRE_RECHARGE_TIME = 5;

const int MAX_POLE_GRAB_TIME = 19;

const int MAX_SCROLL_VIEW = (8<<CSF);

int ck_KeenRunXVels[8] = {0, 0, 4, 4, 8, -4, -4, -8};

int ck_KeenPoleOffs[3] = {-8, 0, 8};


CPlayerLevel::CPlayerLevel(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y,
						std::vector< SmartPointer<CGalaxySpriteObject> > &ObjectPtrs, direction_t facedir,
						CInventory &l_Inventory, stCheat &Cheatmode,
						const size_t offset) :
CPlayerBase(pmap, foeID, x, y, ObjectPtrs, facedir, l_Inventory, Cheatmode),
m_jumpdownfromobject(false),
mPlacingGem(false),
mPoleGrabTime(0)
{
	mActionMap[A_KEEN_STAND] = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
	mActionMap[A_KEEN_QUESTION] = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
	mActionMap[A_KEEN_BORED] = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
	mActionMap[A_KEEN_LOOKUP] = (void (CPlayerBase::*)()) &CPlayerLevel::processLookingUp;
	mActionMap[A_KEEN_LOOKDOWN] = (void (CPlayerBase::*)()) &CPlayerLevel::processLookingDown;
	mActionMap[A_KEEN_SHOOT] = (void (CPlayerBase::*)()) &CPlayerLevel::processShootWhileStanding;
	mActionMap[A_KEEN_SHOOT_UP] = (void (CPlayerBase::*)()) &CPlayerLevel::processShootWhileStanding;
	mActionMap[A_KEEN_SLIDE] = (void (CPlayerBase::*)()) &CPlayerLevel::processSliding;
	mActionMap[A_KEEN_ENTER_DOOR] = static_cast<void (CPlayerBase::*)()>(&CPlayerLevel::processEnterDoor);
	mActionMap[A_KEEN_POLE] = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbingSit;
	mActionMap[A_KEEN_POLE_CLIMB] = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbingUp;
	mActionMap[A_KEEN_POLE_SLIDE] = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbingDown;
	mActionMap[A_KEEN_POLE_SHOOT] = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbingSit;
	mActionMap[A_KEEN_POLE_SHOOTUP] = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbingUp;
	mActionMap[A_KEEN_POLE_SHOOTDOWN] = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbingDown;
	mActionMap[A_KEEN_RUN] = (void (CPlayerBase::*)()) &CPlayerLevel::processRunning;
	mActionMap[A_KEEN_POGO_START] = (void (CPlayerBase::*)()) &CPlayerLevel::processPogoBounce;
	mActionMap[A_KEEN_POGO_UP] = (void (CPlayerBase::*)()) &CPlayerLevel::processPogo;
	mActionMap[A_KEEN_POGO_HIGH] = (void (CPlayerBase::*)()) &CPlayerLevel::processPogo;
	mActionMap[A_KEEN_JUMP] = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
	mActionMap[A_KEEN_JUMP_DOWN] = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
	mActionMap[A_KEEN_FALL] = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
	mActionMap[A_KEEN_JUMP_SHOOT] = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
	mActionMap[A_KEEN_JUMP_SHOOTUP] = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
	mActionMap[A_KEEN_JUMP_SHOOTDOWN] = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
	mActionMap[A_KEEN_HANG] = (void (CPlayerBase::*)()) &CPlayerLevel::processCliffHanging;
	mActionMap[A_KEEN_CLIMB] = (void (CPlayerBase::*)()) &CPlayerLevel::processCliffClimbing;

	m_fire_recharge_time = 0;
	m_EnterDoorAttempt = false;
	m_ptogglingswitch = false;

	m_jumpheight = 0;
	m_climbing = false;
	m_pogotoggle = false;
	m_jumped = false;
	m_hanging = false;

	/*for(size_t add = 0x098C ; add <= 0x3ABB ; add += 0x2 )
	{
		m_Action.spriteLeft = 0;
		m_Action.spriteRight = 0;
		m_Action.setActionFormat(add);
		setActionSprite();

		for(int sp = 130+124 ; sp <= 155+124 ; sp++)
		{
			if( m_Action.spriteLeft == sp || m_Action.spriteRight == sp)
			{
				printf("sprite %i and %i found at %x\n", m_Action.spriteLeft, m_Action.spriteRight, add);
			}
		}
	}*/

	setupGalaxyObjectOnMap(offset, A_KEEN_STAND);

	CSprite &rSprite = g_pGfxEngine->getSprite(sprite);
	performCollisions();
	processMove( 0, rSprite.m_bboxY1-rSprite.m_bboxY2 );
	if(!processActionRoutine())
			exists = false;
}






bool CPlayerLevel::verifyforPole()
{
	// TODO: Something strange here? Ticks?
	if ( mPoleGrabTime < MAX_POLE_GRAB_TIME )
		return false;

	mPoleGrabTime = 0;

	Uint32 l_x = ( getXLeftPos() + getXRightPos() ) / 2;
	l_x = (l_x>>CSF)<<CSF;
	const int l_y_up = ( getYUpPos() ) + (4<<STC);
	const int l_y_down = ( ( getYDownPos() >> CSF ) + 1 ) << CSF;

	const int yDir = (m_playcontrol[PA_Y] < 0) ? -1 : 1;

	// Now check if Player has the chance to climb a pole or something similar
	if( ( yDir < 0 && ( hitdetectWithTileProperty(1, l_x, l_y_up) & 0x7F) == 1 ) ||
		( yDir > 0 && ( hitdetectWithTileProperty(1, l_x, l_y_down) & 0x7F) == 1 ) ) // 1 -> stands for pole Property
	{
		// Move to the proper X Coordinates, so Keen really grabs it!
		moveTo(VectorD2<int>(l_x - (7<<STC), getYPosition()));

		xinertia = 0;

		next.x = 0;
		next.y = 64*yDir;

		// Set Keen in climb mode
		setAction(A_KEEN_POLE);
		m_climbing = true;
		mClipped = false;
		solid = false;
		return true;
	}
	return false;
}


const int POGO_START_INERTIA = 125; // 48 In K5 Disassemble


void CPlayerLevel::processRunning()
{
	prepareToShoot();

	int px = m_playcontrol[PA_X];
	int py = m_playcontrol[PA_Y];

	// Most of the walking routine is done by the action script itself

	// Center the view after Keen looked up or down
	centerView();

	// He could stand again, if player doesn't move the dpad
	if( px == 0 )
	{
		setAction(A_KEEN_STAND);
		yDirection = 0;
		handleInputOnGround();
		return;
	}
	else
	{
		makeWalkSound();
	}


	if ( py == -1)
	{
		if ( verifyforPole() ) return;
	}
	else if ( py == 1)
	{
		if ( verifyforPole() ) return;
	}


	xDirection = 0;
	if( px < 0 )
	{
		xDirection = -1;
	}
	else if( px > 0 )
	{
		xDirection = 1;
	}


	if (state.jumpIsPressed && !state.jumpWasPressed)
	{
		state.jumpWasPressed = true;
		xinertia = xDirection * 16;
		yinertia = -90;
		nextX = nextY = 0;
		state.jumpTimer = 18;
		setAction(A_KEEN_JUMP);
		playSound( SOUND_KEEN_JUMP );
		return;
	}

	if (state.pogoIsPressed && !state.pogoWasPressed)
	{
		state.pogoWasPressed = true;
		xinertia = xDirection * 16;
		yinertia = -POGO_START_INERTIA;
		nextX = 0;
		state.jumpTimer = 24;
		playSound( SOUND_KEEN_POGO );
		setAction(A_KEEN_POGO_START);
		return;
	}


	if (verifyForFalling())
	{
		xinertia = xDirection * 8;
		yinertia = 0;
		setAction(A_KEEN_FALL);
		playSound( SOUND_KEEN_FALL );
		state.jumpTimer = 0;
	}

	if ( (blockedl && xDirection == -1) || (blockedr && xDirection == 1))
	{
		setAction(A_KEEN_STAND);
		yDirection = 0;
	}

	// He could place a gem
	for( Uint32 i=7 ; i<=10 ; i++ )
	{
		const int l_x = getXMidPos();
		const int l_y = getYDownPos()-(3<<STC);

		// This will change the gemholder to a holder with the gem
		if( hitdetectWithTileProperty(i, l_x, l_y) )
		{
			stItemGalaxy &m_Item = m_Inventory.Item;

			if(i == 7 && m_Item.m_gem.red > 0)
				m_Item.m_gem.red--;
			else if(i == 8 && m_Item.m_gem.yellow > 0)
				m_Item.m_gem.yellow--;
			else if(i == 9 && m_Item.m_gem.blue > 0)
				m_Item.m_gem.blue--;
			else if(i == 10 && m_Item.m_gem.green > 0)
				m_Item.m_gem.green--;
			else
				break;

			moveToHorizontal((l_x>>CSF)<<CSF);
			mPlacingGem = true;
			setAction(A_KEEN_SLIDE);
		}
	}

}


void CPlayerLevel::prepareToShoot()
{
	// He could shoot
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		const int newx = getXPosition() + ((xDirection == LEFT) ? -(16<<STC) : (16<<STC));
		const int newy = getYPosition()+(4<<STC);

		const VectorD2<int> newVec(newx, newy);
		tryToShoot(newVec, xDirection, yDirection);

		setAction(A_KEEN_SHOOT);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processShootWhileStanding;
		m_fire_recharge_time = FIRE_RECHARGE_TIME;
		return;
	}
}

void CPlayerLevel::handleInputOnGround()
{
	int px = m_playcontrol[PA_X];
	int py = m_playcontrol[PA_Y];


	if(px)
	{
		setAction(A_KEEN_RUN);
		processRunning();

		nextX = (xDirection * m_Action.velX * g_pTimer->getTicksPerFrame())/4;
		return;
	}

	if( state.jumpIsPressed && !state.jumpWasPressed)
	{
		state.jumpWasPressed = true;
		xinertia = 0;
		yinertia = -90;
		nextY = 0;
		state.jumpTimer = 18;
		setAction(A_KEEN_JUMP);
		playSound( SOUND_KEEN_JUMP );
		return;
	}

	if( state.pogoIsPressed && !state.pogoWasPressed)
	{
		state.pogoWasPressed = true;
		xinertia = 0;
		yinertia = -POGO_START_INERTIA;
		setAction(A_KEEN_POGO_START);
		playSound( SOUND_KEEN_POGO );
		nextY = 0;
		state.jumpTimer = 24;
		return;
	}

	// He could duck or use the pole
	if( py > 0 )
	{
		if(verifyforPole()) return;

		setAction(A_KEEN_LOOKDOWN);
		return;
	}

	// He could press up and do further actions
	if( py < 0 )
	{
		if(!verifyforPole())
		{
			//mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPressUp;
			processPressUp();
		}
	}


}



/// Keen standing routine
void CPlayerLevel::processStanding()
{
	prepareToShoot();

	verifyFalling();

	int px = m_playcontrol[PA_X];
	int py = m_playcontrol[PA_Y];

	if (verifyForFalling())
	{
		xinertia = xDirection * 8;
		yinertia = 0;
		setAction(A_KEEN_FALL);
		playSound( SOUND_KEEN_FALL );
		state.jumpTimer = 0;
	}

	if( px || py || state.jumpIsPressed || state.pogoIsPressed )
	{
		user1 = user2 = 0;
		handleInputOnGround();

		return;
	}

	//TODO: If not on platform
	user1 += g_pTimer->getTicksPerFrame();

	if (user2 == 0 && user1 > 200)
	{
		user2++;
		setAction(A_KEEN_QUESTION);
		user1 = 0;
		return;
	}

	if (user2 == 1 && user1 > 300)
	{
		user2++;
		setAction(A_KEEN_BORED);
		user1 = 0;
		return;
	}

	if (user2 == 2 && user1 > 700)
	{
		user2++;
		setAction(A_KEEN_BOOK_OPEN);
		user1 = 0;
	}

	// Center the view after Keen looked up or down
	centerView();

}





void CPlayerLevel::processLookingDown()
{
	verifyFalling();

	//Try to jump down
	if (state.jumpIsPressed && !state.jumpWasPressed && (blockedd&7) == 1)
	{
		state.jumpWasPressed = true;

		//printf("Tryin' to jump down\n");
		//If the tiles below the player are blocking on any side but the top, they cannot be jumped through
		/*int tile1 = CA_TileAtPos(obj->clipRects.tileXmid, obj->clipRects.tileY2, 1);
		int tile2 = CA_TileAtPos(obj->clipRects.tileXmid, obj->clipRects.tileY2+1, 1);

		if (TI_ForeLeft(tile1) || TI_ForeBottom(tile1) || TI_ForeRight(tile1))
			return;

		if (TI_ForeLeft(tile2) || TI_ForeBottom(tile2) || TI_ForeRight(tile2))
			return;

		#define max(a,b) ((a>b)?a:b)

		int deltay = max(g_pTimer->getTicksPerFrame(),4) << 4;

		//TODO: Moving platforms

		clipRects.unitY2 += deltay;
		posY += deltay;
		nextX = 0;
		nextY = 0;
		g_pSound->play
		setAction(A_KEEN_FALL);
		xinertia = yinertia = 0;*/
		playSound( SOUND_KEEN_FALL );
		return;
	}


	if ( m_playcontrol[PA_Y] <= 0 || m_playcontrol[PA_X] != 0
			|| (state.jumpIsPressed && !state.jumpWasPressed)
			|| (state.pogoIsPressed && !state.pogoWasPressed))
	{
		setAction(A_KEEN_STAND);
		yDirection = 0;
		return;
	}


	if( m_playcontrol[PA_Y]>0 )
	{
		const bool jumpdowntile = canFallThroughTile();
		if ( m_playcontrol[PA_JUMP] > 0 && ( supportedbyobject || jumpdowntile )  )
		{
			m_jumpdownfromobject = supportedbyobject;
			m_jumpdown = jumpdowntile;
			supportedbyobject = false;
			blockedd = false;
			yinertia = 0;
			xinertia = 0;
			setAction(A_KEEN_JUMP_DOWN);
			playSound( SOUND_KEEN_FALL );
		}

		if( m_camera.m_relcam.y < MAX_SCROLL_VIEW )
			m_camera.m_relcam.y += (2<<STC);

		return;
	}

	setAction(A_KEEN_STAND);
}







// This special code is important, so platforms in all cases will catch Keen when he is falling on them
void CPlayerLevel::processMoveBitDown()
{
	for( size_t i = 0 ; i<m_ObjectPtrs.size() ; i++ )
	{
		if(m_ObjectPtrs[i]->hitdetect(*this))
		{
			if( CPlatform *platform = dynamic_cast<CPlatform*>(m_ObjectPtrs[i].get()) )
			{
				platform->getTouchedBy(*this);
			}
		}
	}

	CSpriteObject::processMoveBitDown();
}





void CPlayerLevel::processInput()
{
	CPlayerBase::processInput();

	if(m_playcontrol[PA_Y] >= 0)
		m_EnterDoorAttempt = false;

	state.jumpWasPressed = state.jumpIsPressed;
	state.pogoWasPressed = state.pogoIsPressed;

	state.jumpIsPressed = m_playcontrol[PA_JUMP];
	state.pogoIsPressed = m_playcontrol[PA_POGO];

	if (!state.jumpIsPressed) state.jumpWasPressed = false;
	if (!state.pogoIsPressed) state.pogoWasPressed = false;
}






void CPlayerLevel::tryToShoot( const VectorD2<int> &pos, const int xDir, const int yDir )
{
	if(m_Inventory.Item.m_bullets > 0)
	{
		m_ObjectPtrs.push_back(new CBullet(mp_Map, 0, pos.x, pos.y, xDir, yDir));
		m_Inventory.Item.m_bullets--;
	}
	else
	{
		playSound( SOUND_GUN_CLICK );
	}
}




void CPlayerLevel::shootInAir()
{
	// process simple shooting
	if( m_playcontrol[PA_Y] < 0 )
	{
		setActionForce(A_KEEN_JUMP_SHOOTUP);
		const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYUpPos()-(16<<STC));
		tryToShoot(newVec, 0, -1);
	}
	else if( m_playcontrol[PA_Y] > 0 )
	{
		setActionForce(A_KEEN_JUMP_SHOOTDOWN);
		const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYDownPos());
		tryToShoot(newVec, 0, 1);
	}
	else
	{
		setActionForce(A_KEEN_JUMP_SHOOT);

		const VectorD2<int> newVec(getXPosition() + ((xDirection < 0) ? -(16<<STC) : (16<<STC)),
									getYPosition()+(4<<STC));
		tryToShoot(newVec, xDirection, yDirection);
	}
	m_fire_recharge_time = FIRE_RECHARGE_TIME;
}




bool CPlayerLevel::checkandtriggerforCliffHanging()
{
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	if( m_playcontrol[PA_X]<0 && blockedl )
	{
		bool check_block = TileProperty[mp_Map->at((getXLeftPos()>>CSF)-1, getYUpPos()>>CSF)].bright;
		bool check_block_lower = TileProperty[mp_Map->at((getXLeftPos()>>CSF)-1, (getYUpPos()>>CSF)+1)].bright;
		if(!check_block && check_block_lower &&  mp_processState != (void (CPlayerBase::*)()) &CPlayerLevel::processPogo )
		{
			setAction(A_KEEN_HANG);
			setActionSprite();
			calcBoundingBoxes();
			Uint32 x = (((getXPosition()>>CSF))<<CSF)+(16<<STC);
			Uint32 y = (((getYPosition()>>CSF))<<CSF)+(8<<STC);
			moveTo(x,y);
			solid = false;
			yinertia = 0;
			m_hanging = false;
			return true;
		}
	}
	else if( m_playcontrol[PA_X]>0 && blockedr )
	{
		bool check_block = TileProperty[mp_Map->at((getXRightPos()>>CSF)+1, getYUpPos()>>CSF)].bleft;
		bool check_block_lower = TileProperty[mp_Map->at((getXRightPos()>>CSF)+1, (getYUpPos()>>CSF)+1)].bleft;
		if(!check_block && check_block_lower && mp_processState != (void (CPlayerBase::*)()) &CPlayerLevel::processPogo )
		{
			setAction(A_KEEN_HANG);
			setActionSprite();
			calcBoundingBoxes();
			Uint32 x = (((getXPosition()>>CSF)+1)<<CSF) + (2<<STC);
			Uint32 y = (((getYPosition()>>CSF)+1)<<CSF) - (5<<STC);
			moveTo(x,y);
			solid = false;
			yinertia = 0;
			m_hanging = false;
			return true;
		}
	}
	return false;
}







void CPlayerLevel::processCliffHanging()
{
	// In case you released the direction
	if( m_playcontrol[PA_Y] == 0 && m_playcontrol[PA_X] == 0)
		m_hanging = true;

	if(!m_hanging)
		return;

	// If you want to climb up
	if( ((xDirection == LEFT) && (m_playcontrol[PA_X] < 0)) ||
		((xDirection == RIGHT) && (m_playcontrol[PA_X] > 0))  )
	{
		setAction(A_KEEN_CLIMB);
		m_camera.m_freeze = true;
	}

	// If you want to fall down.
	else if( m_playcontrol[PA_Y] > 0 ||
		((xDirection == LEFT) && (m_playcontrol[PA_X] > 0)) ||
		((xDirection == RIGHT) && (m_playcontrol[PA_X] < 0))  )
	{
		setAction( A_KEEN_FALL );
		playSound( SOUND_KEEN_FALL );
		solid = true;
		const int dx = 8<<STC;
		moveXDir( (xDirection == LEFT) ? dx : -dx, true);
		setActionSprite();
		calcBoundingBoxes();
	}
}






void CPlayerLevel::processCliffClimbing()
{
	const int dy = 24;
	const int dx = dy/3;
	moveUp(dy);
	moveXDir( (xDirection == LEFT) ? -dx : dx, true);

	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	if( getActionStatus(A_KEEN_STAND) )
	{
		const int target_x = getXMidPos()>>CSF;
		const int target_y = getYDownPos()>>CSF;
		const bool noblock = !TileProperty[mp_Map->at(target_x, target_y)].bup;
		if(noblock)
		{
			moveDown(1<<CSF);
			solid = true;
			m_camera.m_freeze = false;
			setActionSprite();
			calcBoundingBoxes();
			setAction(A_KEEN_STAND);
			yDirection = 0;
		}
	}
}





void CPlayerLevel::processMovingHorizontal()
{
	// Verify facing directions
	if(  m_playcontrol[PA_X]<0  ) // left
		xDirection = LEFT;
	else if( m_playcontrol[PA_X]>0  ) // right
		xDirection = RIGHT;

	moveXDir(m_playcontrol[PA_X]>>1);
}


void CPlayerLevel::processPogoCommon()
{
	if (blockedl && xDirection == -1)
		xinertia = 0;
	if (blockedr && xDirection == 1)
		xinertia = 0;


	if (blockedu)
	{
		//TODO: Something to do with poles (push keen into the centre)
		if (blockedu == 17)	//Pole
		{
			//obj->posY -= 32;
			//obj->clipRects.unitY1 -= 32;
			xinertia = 0;
			//obj->posX = (obj->clipRects.tileXmid << 8) - 32;
		}
		else
		{
			playSound( SOUND_KEEN_BUMPHEAD );
			if (blockedu > 1)
			{
				yinertia += 16;
				if (yinertia < 0)
					yinertia = 0;
			}
			else
			{
				if( yinertia < 0 )
				{
					yinertia = 0;
				}
			}
			state.jumpTimer = 0;
		}
	}



	// Houston, we've landed!
	if(blockedd)
	{
		//yinertia = 0; // Not sure if that's correct
		//TODO: Deadly surfaces and fuse breakage.
		if (state.jumpTimer == 0)
		{
			yinertia = -POGO_START_INERTIA;
			playSound( SOUND_KEEN_POGO );
			state.jumpTimer = 24;
			setAction(A_KEEN_POGO_UP);
		}
	}
}


void CPlayerLevel::processPogoBounce()
{
	processPogoCommon();

	yinertia = -POGO_START_INERTIA;
	//nextY = 6 * yinertia;
	state.jumpTimer = 24;

	if( !getActionStatus(A_KEEN_POGO_START) )
		setAction(A_KEEN_POGO_UP);
}



// Here all the pogo code is processed
void CPlayerLevel::processPogo()
{
	processPogoCommon();

	if (!state.jumpTimer)
	{
		if (g_pBehaviorEngine->mDifficulty == EASY)
		{
			performGravityMid();
		}
		else
		{
			performGravityHigh();
		}
	}
	else
	{
		if (state.jumpIsPressed || state.jumpTimer <= 9)
			performGravityLow();
		else
			performGravityHigh();

		if (state.jumpTimer < 2)
			state.jumpTimer = 0;
		else
			state.jumpTimer--;

		if (!state.jumpTimer && m_Action.Next_action )
			//m_Action.setNextActionFormat();
			setAction(A_KEEN_POGO_HIGH);
	}


	if (m_playcontrol[PA_X])
	{
		//if (!xinertia)
		{
			if(m_playcontrol[PA_X]<0)
				xDirection = -1;
			else if(m_playcontrol[PA_X]>0)
				xDirection = 1;
		}
		performPhysAccelHor(xDirection, 48);
	}
	else
	{
		//obj->nextX += obj->velX * CK_GetTicksPerFrame();
		if (xinertia < 0) xDirection = -1;
		else if (xinertia > 0) xDirection = 1;
	}

	if (state.pogoIsPressed && !state.pogoWasPressed)
	{
		state.pogoWasPressed = true;
		setAction(A_KEEN_FALL);
	}

	moveXDir(xinertia);


	// TODO: This is old stuff, needs to be removed
	/*if(!m_playcontrol[PA_POGO])
		m_pogotoggle = false;*/

	// process Shooting in air
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		xinertia = 0;
		shootInAir();
		return;
	}
}


/*

void CK_KeenReadThink(CK_object *obj)
{
	if (IN_GetKeyState(IN_SC_LeftArrow) || IN_GetKeyState(IN_SC_RightArrow))
	{
		obj->currentAction = &CK_ACT_keenStowBook1;
		obj->user1 = obj->user2 = 0;
	}
}

*/


void CPlayerLevel::verifyJumpAndFall()
{
	if ( blockedl && xDirection == -1)
	{
		xinertia = 0;
	}
	if ( blockedr && xDirection == 1)
	{
		xinertia = 0;
	}

	// Did we hit our head on the ceiling?
	if (blockedu)
	{
		//TODO: Something to do with poles (push keen into the centre)
		if (blockedu == 17)	//Pole
		{
			/*obj->posY -= 32;
			obj->clipRects.unitY1 -= 32;*/
			xinertia = 0;
			//obj->posX = (obj->clipRects.tileXmid << 8) - 32;
		}
		else
		{
			playSound( SOUND_KEEN_BUMPHEAD );
			if (blockedu > 1)
			{
				yinertia += 16;
				if (yinertia < 0)
					yinertia = 0;
			}
			else
			{
				if( yinertia<0 )
					yinertia = 0;
			}
			state.jumpTimer = 0;
		}
	}


	// If keen is jumping down, not because he did from an object like a platform,
	// but a tile where Keen can fall through, process this part of code and
	// check if Keen is still jumpinto through any object
	if(!supportedbyobject && m_jumpdown)
	{
		if(!canFallThroughTile())
			m_jumpdown = false;
	}

	// Have we landed?
	if (blockedd)
	{
		//obj->deltaPosY = 0;
		//TODO: Check if deadly.

		//TODO: Check if fuse.

		if (state.jumpTimer == 0) // Or standing on a platform.
		{
			user1 = user2 = 0;	// Being on the ground is boring.
			yDirection = 0;

			//TODO: Finish these
			if( m_playcontrol[PA_X] != 0 )
			{
				setAction(A_KEEN_RUN);
			}
			else
			{
				setAction(A_KEEN_STAND);
			}

		}
	}
	/*else if (deltaPosY > 0)
	{
		int temp6 = obj->clipRects.unitY1 - obj->deltaPosY;
		int temp8 = ((obj->clipRects.unitY1 - 64) & 0xFF00) + 64;
		int temp10 = (temp8 >> 8) -1;

		if (temp6 < temp8 && obj->clipRects.unitY1 >= temp8)
		{
			printf("Preparing to hang!\n");
			if (ck_inputFrame.xDirection == -1)
			{
				int tileX = obj->clipRects.tileX1 - ((obj->rightTI)?1:0);
				int tileY = temp10;
				int upperTile = CA_TileAtPos(tileX, tileY, 1);
				int lowerTile = CA_TileAtPos(tileX, tileY+1, 1);
				printf("[%d,%d]: RightTI: %d, UpperTile = %d [%d], LowerTile = %d, [%d]\n",tileX, tileY, obj->rightTI, upperTile, TI_ForeRight(upperTile), lowerTile, TI_ForeRight(lowerTile));
				if ( (!TI_ForeLeft(upperTile) && !TI_ForeRight(upperTile) && !TI_ForeTop(upperTile) && !TI_ForeBottom(upperTile)) &&
					TI_ForeRight(lowerTile) && TI_ForeTop(lowerTile))
				{
					obj->xDirection = -1;
					obj->clipped = false;
					obj->posX = (obj->posX & 0xFF00) + 128;
					obj->posY = (temp8 - 64);
					obj->velY = obj->deltaPosY = 0;
					CK_SetAction2(obj, &CK_ACT_keenHang1);
					printf("Hung left!\n");
				} else printf("Couldn't hang left!\n");
			}
			else if (ck_inputFrame.xDirection == 1)
			{
				int tileX = obj->clipRects.tileX2 + ((obj->leftTI)?1:0);
				int tileY = temp10;
				int upperTile = CA_TileAtPos(tileX, tileY, 1);
				int lowerTile = CA_TileAtPos(tileX, tileY+1, 1);

				if (!TI_ForeLeft(upperTile) && !TI_ForeRight(upperTile) && !TI_ForeTop(upperTile) && !TI_ForeBottom(upperTile) &&
					TI_ForeLeft(lowerTile) && TI_ForeTop(lowerTile))
				{
					obj->xDirection = 1;
					obj->clipped = false;
					obj->posX = (obj->posX & 0xFF00) + 256;
					obj->posY = (temp8 - 64);
					obj->velY = obj->deltaPosY = 0;
					CK_SetAction2(obj, &CK_ACT_keenHang1);
				}
			}
		}
	}*/

	//RF_AddSpriteDraw(&obj->sde, obj->posX, obj->posY, obj->gfxChunk, 0, obj->zLayer);
}


// Processes the jumping of the player
void CPlayerLevel::processJumping()
{
	verifyJumpAndFall();
	if (state.jumpTimer)
	{
		if(state.jumpTimer > 0 && !m_Cheatmode.jump)
			state.jumpTimer--;

		// Stop moving up if we've let go of control.
		if (!state.jumpIsPressed)
			state.jumpTimer = 0;

		//yinertia = -90;

		moveYDir(yinertia);

	}
	else
	{
		if(g_pBehaviorEngine->mDifficulty == EASY)
		{
			performGravityMid();
		}
		else	// Normal or Hard
		{
			performGravityHigh();
		}

		if( yinertia >0 && !getActionStatus(A_KEEN_FALL) && !getActionStatus(A_KEEN_FALL+1)  )
		{
			m_Action.setNextActionFormat();
		}
	}


	//Move horizontally
	if ( m_playcontrol[PA_X] != 0 )
	{
		xDirection = (m_playcontrol[PA_X] < 0) ? -1 : 1;
		performPhysAccelHor(xDirection*4, 48);
	}
	else performPhysDampHorz();

	moveXDir(xinertia);

	if (state.pogoIsPressed && !state.pogoWasPressed)
	{
		state.pogoWasPressed = true;
		setAction(A_KEEN_POGO_UP);
		state.jumpTimer = 0;
		return;
	}

	if (m_playcontrol[PA_Y] < 0)
	{
		verifyforPole();
	}


	if( m_Cheatmode.jump && (state.jumpIsPressed && !state.jumpWasPressed) )
	{
			state.jumpWasPressed = true;
			yinertia = -90;
			state.jumpTimer = 18;
			setAction(A_KEEN_JUMP);
			return;
	}

	if(checkandtriggerforCliffHanging())
		return;

	// process Shooting in air
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		shootInAir();
}






bool CPlayerLevel::canFallThroughTile()
{
	const int &tile = mp_Map->getPlaneDataAt(1, getXMidPos(), getYDownPos()+(2<<STC));
	const CTileProperties &TileProp = g_pBehaviorEngine->getTileProperties(1)[tile];
	return ( TileProp.bdown == 0 && TileProp.bup != 0 );
}






void CPlayerLevel::processLookingUp()
{
	// While looking up, Keen could shoot up!
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		setActionForce(A_KEEN_SHOOT_UP);
		const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYUpPos()-(16<<STC));
		tryToShoot(newVec, 0, -1);
		m_fire_recharge_time = FIRE_RECHARGE_TIME;
		return;
	}

	if( m_camera.m_relcam.y > -MAX_SCROLL_VIEW )
		m_camera.m_relcam.y -= (2<<STC);

	if( m_playcontrol[PA_Y]<0 )
		return;

	setAction(A_KEEN_STAND);
	yDirection = 0;
}







// Processes the exiting of the player. Here all cases are held
void CPlayerLevel::processExiting()
{
	Uint32 x = getXMidPos();
	if( ((mp_Map->m_width-2)<<CSF) < x || (2<<CSF) > x )
	{
		CEventContainer& EventContainer = g_pBehaviorEngine->m_EventList;
		EventContainer.add( new EventExitLevel(mp_Map->getLevel(), true) );
	}
}








#define		MISCFLAG_SWITCHPLATON 5
#define 	MISCFLAG_SWITCHPLATOFF 6
#define		MISCFLAG_SWITCHBRIDGE 15
#define		MISCFLAG_DOOR		2
#define		MISCFLAG_KEYCARDDOOR		32

void CPlayerLevel::processPressUp() {

	std::vector<CTileProperties> &Tile = g_pBehaviorEngine->getTileProperties(1);
	const int x_left = getXLeftPos();
	const int x_right = getXRightPos();
	const int x_mid = (x_left+x_right)/2;
	const int up_y = getYUpPos()+(3<<STC);

	const Uint32 tile_no = mp_Map->getPlaneDataAt(1, x_mid, up_y);
	int flag = Tile[tile_no].behaviour;

	// pressing a switch
	if (flag==MISCFLAG_SWITCHPLATON || flag == MISCFLAG_SWITCHPLATOFF ||
		flag == MISCFLAG_SWITCHBRIDGE)
	{
		playSound( SOUND_GUN_CLICK );
		//setAction(A_KEEN_SLIDE);
		if(flag == MISCFLAG_SWITCHBRIDGE)
		{
			Uint32 newtile;
			if(Tile[tile_no+1].behaviour == MISCFLAG_SWITCHBRIDGE)
				newtile = tile_no+1;
			else
				newtile = tile_no-1;

			mp_Map->setTile( x_mid>>CSF, up_y>>CSF, newtile, true, 1); // Wrong tiles, those are for the info plane
			PressBridgeSwitch(x_mid, up_y);
		}
		else
		{
			const Uint32 newtile = (flag == MISCFLAG_SWITCHPLATON) ? tile_no+1 : tile_no-1 ;
			mp_Map->setTile( x_mid>>CSF, up_y>>CSF, newtile, true, 1); // Wrong tiles, those are for the info plane
			PressPlatformSwitch(x_mid, up_y);
		}
		setAction(A_KEEN_SLIDE);
		m_timer = 0;
		return;
	 }
/*		var2 = o->boxTXmid*256-64;
		if (o->xpos == var2) {
			setAction(ACTION_KEENENTERSLIDE);
			setAction(A_KEEN_SLIDE);
		} else {
			o->time = var2;
			//setAction(ACTION_KEENPRESSSWITCH);
			setAction(ACTION_KEENENTERSLIDE);
		}
		EnterDoorAttempt = 1;
		return 1;
	} */

	int flag_left = Tile[mp_Map->getPlaneDataAt(1, x_left, up_y)].behaviour;

	// entering a door

	if ( !m_EnterDoorAttempt && (flag_left == MISCFLAG_DOOR || flag_left == MISCFLAG_KEYCARDDOOR) )
	{
		//int var2 = mid_x * 256+96;
		int flag_right = Tile[mp_Map->getPlaneDataAt(1, x_left+(1<<CSF), up_y)].behaviour;
		//if (flag2 == MISCFLAG_DOOR || flag2 == MISCFLAG_KEYCARDDOOR) var2-=256;
		//if (getXPosition() == var2) {
		if(flag_right == MISCFLAG_DOOR || flag_right == MISCFLAG_KEYCARDDOOR) {
			/*if (flag == MISCFLAG_KEYCARDDOOR) {
				if (security_card) {
					security_card = 0;
					SD_PlaySound(SOUND_OPENSECURITYDOOR);
					GetNewObj(0);
					new_object->xpos = o->boxTXmid-2;
					new_object->ypos = o->boxTY2-4;
					new_object->active = 2;
					new_object->clipping = 0;
					new_object->type = 1;
					new_object->action = ACTION_SECURITYDOOROPEN;
					check_ground(new_object, ACTION_SECURITYDOOROPEN);
					o->action = ACTION_KEENENTERDOOR0;
					o->int16 = 0;
					return 1;
				} else {
					SD_PlaySound(SOUND_NOOPENSECURITYDOOR);
					o->action = ACTION_KEENSTAND;
					EnterDoorAttempt = 1;
					return 0;
				}
			} else {*/
				setAction(A_KEEN_ENTER_DOOR);
				setActionSprite();
				CSprite &rSprite = g_pGfxEngine->getSprite(sprite);

				// Here the Player will be snapped to the center

				const int x_l = (x_left>>CSF);
				const int x_r = x_l+1;
				const int x_mid = ( ((x_l+x_r)<<CSF) - (rSprite.getWidth()<<STC)/2 )/2;

				moveToHorizontal(x_mid);

				m_EnterDoorAttempt = true;
				return;
				//PlayLoopTimer = 110;
				//o->action = ACTION_KEENENTERDOOR1
				//o->int16 = 0;
				//if (*MAPSPOT(o->boxTXmid, o->boxTY1, INFOPLANE) == 0) sub_1FE94();
			//}
		}// else {
			//o->time = var2;
			//o->action = ACTION_KEENENTERSLIDE;
		//}
		//EnterDoorAttempt = 1;
	}

	// If the above did not happen, then just look up
	setAction(A_KEEN_LOOKUP);
}













void CPlayerLevel::processSliding()
{
	if(mPlacingGem)
	{
		if(m_timer == 0 || m_timer == 5 || m_timer == 8)
		{
			openDoorsTile();
		}
	}

	if(m_timer < 10)
	{
		m_timer++;
		return;
	}


	m_timer = 0;
	setAction(A_KEEN_STAND);

	if(mPlacingGem)
	{
		int lx = getXMidPos();
		int ly = getYDownPos()-(3<<STC);

		Uint32 tileno = mp_Map->getPlaneDataAt(1, lx, ly);
		mp_Map->setTile(lx>>CSF, ly>>CSF, tileno+18, true, 1);
		mPlacingGem = false;
		playSound( SOUND_DOOR_OPEN );
	}
}







void CPlayerLevel::processEnterDoor()
{
	moveUp(16);

	if(!getActionStatus(A_KEEN_STAND))
		return;

	setAction(A_KEEN_STAND);
	yDirection = 0;

	int xmid = getXMidPos();
	int y1 = getYMidPos();

	Uint32 t = mp_Map->getPlaneDataAt(2, xmid, y1);
	if (t == 0) {
		//level_state = 13;
		//o->action = ACTION_KEENENTEREDDOOR;
		// TODO: Figure out what this does
		return;
	}

	if (t == 0xB1B1) {
		//level_state = 2;
		//o->action = ACTION_KEENENTEREDDOOR;
		// TODO: Figure out what this does
		return;
	}

	const int ypos = ((t%256 - 1))<<CSF;
	const int xpos = (t >> 8)<<CSF;

	VectorD2<int> new_pos(xpos, ypos);
	moveToForce(new_pos);
	new_pos.x += ((m_BBox.x2-m_BBox.x1)/2);
	new_pos.y += ((m_BBox.y2-m_BBox.y1)/2);
	m_camera.setPosition(new_pos);

	//o->ypos = TILE2MU(*t%256 - 1) + 15;
	//o->xpos = (*t >> 8 << 8);
	//o->int16 = 1;
	//o->clipping = 0;
	//set_sprite_action(o, (o->action)->next);
	//o->clipping = 1;
	//sub_183F1(o);
	return;

}










/**
 * This function checks whether a bridge must be opened or closed and does this kind of work
 * I'm not really happy with that part of the code and I know that it works for Keen 4. Not sure about the
 * other episodes, but it's well though and should...
 */


void CPlayerLevel::PressBridgeSwitch(const Uint32 lx, const Uint32 ly)
{
	Uint32 targetXY = mp_Map->getPlaneDataAt(2, lx, ly);

	Uint32 newX = targetXY >> 8;
	Uint32 newY = targetXY & 0xFF;

	const int start_tile = mp_Map->getPlaneDataAt(1, newX<<CSF, newY<<CSF)-1;
	const int end_tile = start_tile+3;

	/// We found the start of the row, that need to be changed.
	/// Let apply it to the rest of the bridge
	// Apply to the borders

	// bridge opened or closed?
	const bool b_opened = (start_tile%8 < 4) ?true : false;

	int x = newX;
	for(int t = start_tile ;  ; x++ )
	{
		// Now decide whether the tile is a piece or borders of the bridge
		const Uint32 type = t%18;

		if(type < 16)
		{
			if(b_opened)
				t += 4;
			else
				t -= 4;
		}
		else
		{
			// It is just a normal piece remove
			t = (t/18)*18;
			if(b_opened)
				t+=16;
			else
				t+=17;
		}
		const Uint32 NewTile = t;
		t = mp_Map->getPlaneDataAt(1, x<<CSF, newY<<CSF);

		mp_Map->setTile(x-1, newY, NewTile, true, 1);
		mp_Map->setTile(x-1, newY+1, NewTile+18, true, 1);

		if(t == end_tile)
		{
			if(t%8 < 4)
				// This bridge is opened, close it!
				t += 4;
			else
				// This bridge is closed, open it!
				t -= 4;

			Uint32 new_lasttile = end_tile;
			if(b_opened)
				new_lasttile += 4;
			else
				new_lasttile -= 4;

			mp_Map->setTile(x-1, newY+1, new_lasttile+17, true, 1);
			mp_Map->setTile(x, newY, new_lasttile, true, 1);
			mp_Map->setTile(x, newY+1, new_lasttile+18, true, 1);
			break;
		}
	}

	return;
}






/**
 * This function will put/release the blockers of some platforms used in Keen Galaxy
 */
void CPlayerLevel::PressPlatformSwitch(const Uint32 lx, const Uint32 ly)
{
	Uint32 targetXY = mp_Map->getPlaneDataAt(2, lx, ly);

	Uint32 newX = targetXY >> 8;
	Uint32 newY = targetXY & 0xFF;

	if(mp_Map->getPlaneDataAt(2, newX<<CSF, newY<<CSF) == 31)
		mp_Map->setTile(newX, newY, 0, true, 2);
	else
		mp_Map->setTile(newX, newY, 31, true, 2);

	return;
}



void CPlayerLevel::openDoorsTile()
{
	int lx = getXMidPos();
	int ly = getYDownPos()-(3<<STC);
	Uint32 targetXY = mp_Map->getPlaneDataAt(2, lx, ly);

	Uint32 newX = targetXY >> 8;
	Uint32 newY = targetXY & 0xFF;
	Uint32 tileno, next_tileno;

	do
	{
		tileno = mp_Map->getPlaneDataAt(1, newX<<CSF, newY<<CSF);
		mp_Map->setTile(newX, newY, tileno+1, true, 1);
		newY++;
		next_tileno = mp_Map->getPlaneDataAt(1, newX<<CSF, newY<<CSF);
	}while(next_tileno == tileno || next_tileno == tileno+18 || next_tileno == tileno+2*18);
}


/*

void CK_KeenHangThink(CK_object *obj)
{
	printf("Just hangin'...\n");
	if (ck_inputFrame.yDirection == -1 || ck_inputFrame.xDirection == obj->xDirection)
	{
		printf("Goin' up!\n");
		obj->currentAction = &CK_ACT_keenPull1;

		obj->clipped = false;

		if(obj->xDirection == 1)
		{
			obj->nextY = -256;
		}
		else
		{
			obj->nextY = -128;
		}
		//TODO: Set keen->zlayer 3

		//if (obj->xDirection == 1)
		//{


	}
	else if (ck_inputFrame.yDirection == 1 || (ck_inputFrame.xDirection && ck_inputFrame.xDirection != obj->xDirection))
	{
		printf("Goin' Down!\n");
		// Drop down.
		obj->currentAction = &CK_ACT_keenFall1;
		obj->clipped = true;
	}
}

void CK_KeenPullThink1(CK_object *obj)
{
	if (obj->xDirection == 1)
		obj->nextX = 128;
	else
		obj->nextY = -128;
}

void CK_KeenPullThink2(CK_object *obj)
{
	printf("PullThink2\n");
	obj->nextX = obj->xDirection * 128;
	obj->nextY = -128;
}

void CK_KeenPullThink3(CK_object *obj)
{
	printf("PullThink3\n");
	obj->nextY = -128;
}

void CK_KeenPullThink4(CK_object *obj)
{
	printf("PullThink4\n");
	obj->nextY = 128;
	obj->clipped = true;
	obj->zLayer = 1;
}

 */


void CPlayerLevel::performPoleHandleInput()
{
	const int px = m_playcontrol[PA_X];
	const int py = m_playcontrol[PA_Y];

	if ( px )
		xDirection = (px>0) ? 1 : -1;

	// Shooting things. *ZAP!*
	if( py < 0 )
	{
		// First check player pressed shoot button
		if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		{
			m_fire_recharge_time = FIRE_RECHARGE_TIME;
			setActionForce(A_KEEN_POLE_SHOOTUP);
			const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYUpPos()-(16<<STC));
			tryToShoot(newVec, 0, -1);
			return;
		}

	}
	else if( py > 0 )
	{
		// First check player pressed shoot button
		if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		{
			m_fire_recharge_time = FIRE_RECHARGE_TIME;
			setActionForce(A_KEEN_POLE_SHOOTDOWN);
			const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYDownPos());
			tryToShoot(newVec, 0, 1);
			return;
		}
	}
	else
	{
		// First check player pressed shoot button
		if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		{
			m_fire_recharge_time = FIRE_RECHARGE_TIME;
			setActionForce(A_KEEN_POLE_SHOOT);
			const VectorD2<int> newVec(getXPosition() + ((xDirection == LEFT) ? -(16<<STC) : (16<<STC)),
										getYPosition()+(4<<STC));
			tryToShoot(newVec, xDirection, yDirection);
			return;
		}

		setAction(A_KEEN_POLE);
		yDirection = 0;
	}


	if (state.jumpIsPressed && !state.jumpWasPressed)
	{
		state.jumpWasPressed = false;
		//TODO: Play A sound!
		xinertia = ck_KeenPoleOffs[px+1];
		yinertia = -80;
		state.jumpTimer = 10;
		setAction(A_KEEN_JUMP);
		playSound( SOUND_KEEN_JUMP );
		yDirection = 1;
		m_climbing = false;
		m_jumped = true;
		mPoleGrabTime = 0;
		solid = true;
		state.poleGrabTime = 0;
	}

	return;
}


void CPlayerLevel::processPoleClimbingSit()
{
	const int px = m_playcontrol[PA_X];
	const int py = m_playcontrol[PA_Y];

	Uint32 l_x = ( getXLeftPos() + getXRightPos() ) / 2;
	Uint32 l_y_up = getYUpPos();
	Uint32 l_y_down = getYDownPos();


	if ( py > 0 )
	{
		setAction(A_KEEN_POLE_SLIDE);
		yDirection = 1;
		processPoleClimbingDown();
		return;
	}
	else if ( py < 0 )
	{

		// Check for the upper side and don't let him move if the pole ends
		if( hitdetectWithTileProperty(1, l_x, l_y_up) )
		{
			setAction(A_KEEN_POLE_CLIMB);
			yDirection = UP;
		}
		else
		{
			setAction(A_KEEN_POLE);
			yDirection = 0;
		}

		return;
	}


	if ( px )
	{

		// This will check three points and avoid that keen falls on sloped tiles
		const int fall1 = mp_Map->getPlaneDataAt(1, l_x, l_y_down+(1<<CSF));
		const CTileProperties &TileProp1 = g_pBehaviorEngine->getTileProperties(1)[fall1];
		const bool leavePole = (TileProp1.bup != 0);

		if ( leavePole )
		{
			//playSound( SOUND_KEEN_FALL );

			state.jumpWasPressed = false;
			state.jumpIsPressed = true;
			//TODO: Play A sound!

			int dir = 1;
			if(px < 0)
				dir = 0;
			else if(px > 0)
				dir = 2;

			xinertia = ck_KeenPoleOffs[dir];
			yinertia = -20;
			state.jumpTimer = 10;
			solid = true;
			setAction(A_KEEN_JUMP);
			yDirection = 1;
			m_climbing = false;
			m_jumped = true;
			mPoleGrabTime = 0;
			state.poleGrabTime = 0;

			return;
		}

	}

	performPoleHandleInput();
}



void CPlayerLevel::processPoleClimbingUp()
{
	Uint32 l_x = ( getXLeftPos() + getXRightPos() ) / 2;
	Uint32 l_y_up = getYUpPos();

	/*if ((blockedu & 127) != 1)
	{
		setAction(A_KEEN_POLE);
		//processPoleClimbingSit();
		//return;
	}*/

	// Check for the upper side and don't let him move if the pole ends
	if( hitdetectWithTileProperty(1, l_x, l_y_up) )
	{
		setAction(A_KEEN_POLE_CLIMB);
		yDirection = UP;
	}
	else
	{
		setAction(A_KEEN_POLE);
		yDirection = 0;
	}

	performPoleHandleInput();
}



void CPlayerLevel::processPoleClimbingDown()
{
	Uint32 l_x = ( getXLeftPos() + getXRightPos() ) / 2;
	Uint32 l_y_up = getYUpPos()+(16<<STC);
	Uint32 l_y_down = getYDownPos();

	if(!hitdetectWithTileProperty(1, l_x, l_y_down))
		solid = true;
	else
		solid = false;

	// Check for the and upper and lower side, upper because the hand can touch the edge in that case
	const bool up = hitdetectWithTileProperty(1, l_x, l_y_up);
	if( up && !blockedd )
	{
		// Slide down if there is more of the pole
		setAction(A_KEEN_POLE_SLIDE);
		yDirection = DOWN;
	}
	else
	{
		// Fall down if there isn't any pole to slide down
		m_climbing = false;
		yDirection = 0;
		yinertia = 0;
		solid = true;

		const bool down = mp_Map->at(l_x>>CSF, l_y_down>>CSF);

		// Check if keen is trying to climb through the floor.
		// It's quite a strange clipping error if he succeeds.
		if(!(blockedd & 127) && !down)
		{
			state.jumpTimer = 0;
			xinertia = ck_KeenPoleOffs[xDirection + 1];
			yinertia = 0;

			setAction(A_KEEN_FALL);
			playSound( SOUND_KEEN_FALL );
		}
		else
		{
			blockedd = true;
			moveUp(1<<CSF);
			moveDown(1<<CSF);

			setAction(A_KEEN_STAND);
			yDirection = 0;

			/*int yReset = -(obj->clipRects.unitY2 & 255) - 1;
			obj->posY += yReset;
			obj->clipRects.unitY2 += yReset;
			obj->clipRects.tileY2 += -1;
			obj->clipped = true;*/
			setAction(A_KEEN_LOOKDOWN);
			//CK_SetAction2(obj, &CK_ACT_keenLookDown1);

		}
	}

	if (m_playcontrol[PA_Y] == 0)
	{
		setAction(A_KEEN_POLE);
		yDirection = 0;
	}
	else if (m_playcontrol[PA_Y] < 0)
	{
		setAction(A_KEEN_POLE_CLIMB);
		yDirection = -1;
	}

	performPoleHandleInput();

}



void CPlayerLevel::processShootWhileStanding()
{
	// while until player releases the button and get back to stand status
	if( !m_playcontrol[PA_FIRE] )
	{
		yDirection = 0;
		if(getActionNumber(A_KEEN_SHOOT_UP))
			setAction(A_KEEN_LOOKUP);
		else
			setAction(A_KEEN_STAND);
	}
}

void CPlayerLevel::verifyFalling()
{
	if (CSpriteObject::verifyForFalling())
	{
		xinertia = xDirection * 16;
		yinertia = 0;
		setAction(A_KEEN_FALL);
		playSound( SOUND_KEEN_FALL );
		state.jumpTimer = 0;
	}
}




// Falling code
void CPlayerLevel::processFalling()
{
	// If keen is jumping down, not because he did from an object like a platform,
	// but a tile where Keen can fall through, process this part of code and
	// check if Keen is still jumpinto through any object
	if(!supportedbyobject && m_jumpdown)
	{
		if(!canFallThroughTile())
			m_jumpdown = false;
	}

	if(blockedd)
	{
		setAction(A_KEEN_STAND);
		yDirection = 0;
	}

	processMovingHorizontal();

	// If Jump mode is enabled he can jump again
	// This will cancel the pole process and make Keen jump
	if( m_Cheatmode.jump && m_playcontrol[PA_JUMP] > 0 )
	{
		setAction(A_KEEN_JUMP);
		m_climbing = false;
		m_jumped = true;
		yinertia = 0;
		yDirection = 0;
		return;
	}

	/// While falling Keen could switch to pogo again anytime
	// but first the player must release the pogo button
	if( !m_playcontrol[PA_POGO] )
		m_pogotoggle = false;

	// Now we can check if player wants to use it again
	if( !m_pogotoggle && m_playcontrol[PA_POGO] )
	{
		m_jumpheight = 0;
		yinertia = 0;
		setAction(A_KEEN_POGO_START);
		m_pogotoggle = true;
	}

	// Check if keen should stick to the pole
	if( m_playcontrol[PA_Y] < 0 )
	{
		verifyforPole();
	}

	// Check Keen could hang on a cliff and do so if possible
	if(checkandtriggerforCliffHanging())
		return;
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		shootInAir();

}



void CPlayerLevel::centerView()
{
	// If keen looked up or down, this will return the camera to initial position
	if( m_camera.m_relcam.y < 0 )
	{
		m_camera.m_relcam.y += (4<<STC);
		if( m_camera.m_relcam.y > 0 )
			m_camera.m_relcam.y = 0;
	}
	else if( m_camera.m_relcam.y > 0 )
	{
		m_camera.m_relcam.y -= (4<<STC);
		if( m_camera.m_relcam.y < 0 )
			m_camera.m_relcam.y = 0;
	}
}



void CPlayerLevel::process()
{
	if(!m_dying)
	{
		processInput();

		// If no clipping was triggered change solid state of keen
		if(m_Cheatmode.noclipping)
		{
			solid = !solid;
			m_Cheatmode.noclipping = false;
		}

		if(supportedbyobject)
		{
			blockedd = true;
		}
	}

	if ( mPoleGrabTime < MAX_POLE_GRAB_TIME )
		 mPoleGrabTime++;

	(this->*mp_processState)();

	// make the fire recharge time decreased if player is not pressing firing button
	if(m_fire_recharge_time && !m_playcontrol[PA_FIRE])
	{
		m_fire_recharge_time--;
	}

	processLevelMiscFlagsCheck();

	if(!processActionRoutine())
			exists = false;


	if(!m_dying)
	{
		processExiting();

		m_camera.process();
		m_camera.processEvents();

		performCollisions();

		void (CPlayerBase::*PogoProcess)() = static_cast<void (CPlayerBase::*)()>(&CPlayerLevel::processPogo);
		void (CPlayerBase::*FallProcess)() = static_cast<void (CPlayerBase::*)()>(&CPlayerLevel::processFalling);

		// It's not always desired to push out
		if( (mp_processState != PogoProcess) &&
			(mp_processState != FallProcess) )
		{
			processPushOutCollision();
		}
	}
}


}
