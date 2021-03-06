/*
 * CMenuController.h
 *
 *  Created on: 19.02.2012
 *      Author: gerstrong
 */

#include "CBaseMenu.h"
#include "engine/CEvent.h"
#include "CSingleton.h"

#ifndef CMENUCONTROLLER_H_
#define CMENUCONTROLLER_H_


#define gpMenuController CMenuController::Get()


/**
 * Events
 */


struct OpenMenuEvent : CEvent
{
	OpenMenuEvent(CBaseMenu* menuDlgPtr) : mMenuDialogPointer(menuDlgPtr) {};

	SmartPointer<CBaseMenu> mMenuDialogPointer;
};

struct CloseMenuEvent : CEvent
{};

struct CloseAllMenusEvent : CEvent
{};

/**
 * Class Declaration
 */


class CMenuController : public CSingleton<CMenuController>
{

public:
	CMenuController() : mOpenedGamePlay(false),
                          mLocked(false) {}

	void emptyMenuStack();

	void openMainMenu();

	void process();

	bool active()
	{	return	!mMenuStack.empty();	}

	void lock(const bool value)
	{	mLocked = value;	}

	bool mOpenedGamePlay;

private:

	void popBackMenu();

	SmartPointer<CBaseMenu> mpMenu;
	std::list< SmartPointer<CBaseMenu> > mMenuStack;

	bool mLocked;
};

#endif /* CMENUCONTROLLER_H_ */
