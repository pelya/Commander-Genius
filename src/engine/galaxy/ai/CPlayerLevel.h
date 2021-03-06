/*
 * CPlayerLevel.h
 *
 *  Created on: 06.08.2010
 *      Author: gerstrong
 */

#ifndef CPLAYERLEVEL_H_
#define CPLAYERLEVEL_H_

#include "CPlayerBase.h"

namespace galaxy {

enum PLAYER_LEVEL_ACTION
{
A_KEEN_STAND = 0,
A_KEEN_BORED = 1,
A_KEEN_QUESTION	 = 2,
A_KEEN_MOON	 = 8,
A_KEEN_BOOK_OPEN = 11,
A_KEEN_BOOK_READ = 15,
A_KEEN_BOOK_CLOSE = 18,
A_KEEN_LOOKUP = 21,
A_KEEN_LOOKDOWN	 = 23,
A_KEEN_ACTION_1	 = 26,
A_KEEN_ACTION_2	 = 27,
A_KEEN_ACTION_3 = 28,
A_KEEN_SHOOT = 33,
A_KEEN_SHOOT_UP = 35,
A_KEEN_ACTION_4 = 37,
A_KEEN_SLIDE = 40,
A_KEEN_ENTER_DOOR = 41,
A_KEEN_POLE	 = 46,
A_KEEN_POLE_CLIMB = 47,
A_KEEN_POLE_SLIDE = 50,
A_KEEN_POLE_SHOOT = 54,
A_KEEN_POLE_SHOOTUP = 56,
A_KEEN_POLE_SHOOTDOWN = 58,
A_KEEN_RUN = 60,
A_KEEN_POGO_START = 64,
A_KEEN_POGO_UP = 65,
A_KEEN_POGO_HIGH = 66,
A_KEEN_JUMP	 = 67,
A_KEEN_JUMP_DOWN = 68,
A_KEEN_FALL	 = 69,
A_KEEN_JUMP_SHOOT = 71,
A_KEEN_JUMP_SHOOTUP	 = 74,
A_KEEN_JUMP_SHOOTDOWN = 77,
A_KEEN_HANG	 = 80,
A_KEEN_CLIMB = 82
};


struct KeenState
{
	KeenState() :
		jumpTimer(0), poleGrabTime(0),
		jumpIsPressed(false), jumpWasPressed(false),
		pogoIsPressed(false), pogoWasPressed(false)
		{}

	int jumpTimer;
	int poleGrabTime;
	bool jumpIsPressed;
	bool jumpWasPressed;
	bool pogoIsPressed;
	bool pogoWasPressed;
};


class CPlayerLevel : public CPlayerBase
{
public:
	CPlayerLevel(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y,
			std::vector< SmartPointer<CGalaxySpriteObject> > &ObjectPtrs, direction_t facedir,
			CInventory &l_Inventory, stCheat &Cheatmode,
			const size_t offset);


	void prepareToShoot();
	/**
	 * \brief	handles the input when keen standing on ground.
	 */
	void handleInputOnGround();


	/**
	 * \description Read the Input of the Player and sets the variables accordingly
	 * 				This function is overloaded. Most of the process is done by the
	 * 				CPlayerBase class.
	 */
	void processInput();


	// Checks if player can fall through a tile
	bool canFallThroughTile();

	/**
	 *  \brief This will center Keens view after he looked up or down.
	 */
	void centerView();


	/**
	 * Input handles
	 */

	/**
	 * \description This is the main process cycle
	 */
	void process();



	/**
	 * \description Simple process called while Keen is standing
	 */
	void processStanding();



	/**
	 * \description Verify if Keen can stick to the pole and climb on it!
	 */
	bool verifyforPole();



	/**
	 * \description This will make keen stand in a pose with the gun and shoot if he has bullets
	 */
	void processShootWhileStanding();



	/**
	 * \description Simple process called while Keen is running
	 */
	void processRunning();



	/**
	 * \description verifies whether there is a cliff and Keen can hang on it.
	 * 				If that the case, the function places him properly
	 * 				and set the process to processCliffHanging
	 * \return 		if true, the cliff was detected and Keen will go into cliff
	 * 				hanging mode.
	 */
	bool checkandtriggerforCliffHanging();


	/*
	 * \description Keen hangs here on a cliff and it might decide
	 * 				whether to fall or climb up
	 */
	void processCliffHanging();


	/*
	 * \description This will make Keen climb the cliff up and stand
	 * 				on the ground. After that it returns to stand action
	 */
	void processCliffClimbing();



	/**
	 * \description Simple function which just process moving in x direction
	 * 				of the player. It is called by other functions like
	 * 				processJumping or processFalling. processRunning not,
	 * 				because it has to stop running when players gets blocked
	 * 				to left or right.
	 */
	void processMovingHorizontal();



	/**
	 * \brief Checks if Keen must fall.
	 */
	void verifyFalling();



	/**
	 * \description Simple process called while Keen is falling down
	 */
	void processFalling();



	/**
	 * \description Firing Routine for Keen
	 */
	void processFiring();


	/**
	 * This function is processed by falling and jumping. It just manages the Shoot in air routines
	 */
	void shootInAir();

	void verifyJumpAndFall();

	void processJumping();

	void processPogoCommon();
	void processPogoBounce();
	void processPogo();

	void processLooking();


	/**
	 * \brief		This function will try to spawn a shot. If Keen has bullets it will spawn
	 * 				otherwise, the empty clicking sound is heard
	 * \param	pos	Coordinates where the shot should be spawned
	 */
	void tryToShoot( const VectorD2<int> &pos, const int xDir, const int yDir );



	/**
	 * This function will be processed while Keen is looking up
	 */
	void processLookingUp();

	/**
	 * This function will process everything that happens when player presses up
	 */
	void processPressUp();

	/**
	 * This function will ensure that Keen keeps ducking
	 */
	void processLookingDown();

	/**
	 * This function will do the cycle when Keen is on the pole
	 */

	void performPoleHandleInput();

	void processPoleClimbingSit();

	void processPoleClimbingUp();

	void processPoleClimbingDown();

	void processExiting();

	/**
	 * This function will be process will keen is pressing a switch. Placing gem uses
	 * a different process called processPlaceGem()
	 * He just slides to that item or tile
	 */
	void processSliding();

	/**
	 * This function will process the going into a door
	 */
	void processEnterDoor();


	/**
	 * This function will open/close bridges in Keen Galaxy
	 * \param lx CSFed Coordinate where the switch has been triggered
	 */
	void PressBridgeSwitch(const Uint32 lx, const Uint32 ly);

	void PressPlatformSwitch(const Uint32 lx, const Uint32 ly);
	void openDoorsTile();

	// Checks if Keen is using the pogo. Skypest needs to know that,
	// because it might get squashed.
	bool isPogoing()
	{
		return (mp_processState == (void (CPlayerBase::*)()) &CPlayerLevel::processPogo );
	}



	bool m_jumpdownfromobject;
	bool mPlacingGem;

private:

	/** \brief Special code when Keen moving down... */
	void processMoveBitDown();

	KeenState state;

	bool m_ptogglingswitch;
	int m_jumpheight;

	bool m_pogotoggle;

	int m_fire_recharge_time;
	bool m_EnterDoorAttempt;
	bool m_hanging;
	int mPoleGrabTime;

};

}

#endif /* CPLAYERLEVEL_H_ */
