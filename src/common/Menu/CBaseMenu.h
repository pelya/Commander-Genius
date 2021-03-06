/*
 * CBaseMenu.h
 *
 *  Created on: 28.11.2009
 *      Author: gerstrong
 */

#ifndef CBASEMENU_H_
#define CBASEMENU_H_

const int NO_SELECTION = -1;

#include "SmartPointer.h"
#include "gui/CGUIDialog.h"
#include "gui/CGUIButton.h"
#include <list>

class CBaseMenu
{
public:

	enum Property
	{
		CLOSEABLE,
		CANGOBACK
	};


	CBaseMenu( const CRect<float>& rect );

	virtual ~CBaseMenu() { };

	virtual void init() {};

	void select(const size_t value);

	virtual void release() {};

	void setMenuLabel(const std::string &label);

	// Processes the stuff that the menus have in common
	virtual void process();

	virtual void sendEvent(SmartPointer<CEvent> command)
	{
		mpMenuDialog->sendEvent(command);
	}

	void setProperty( const Property newProperty )
	{
		mpReturnButton->setText( newProperty == CLOSEABLE ? "x" : "\025" );
	}


protected:
	SmartPointer<CGUIDialog> mpMenuDialog;
	CGUIButton *mpReturnButton;
	std::list< SmartPointer<CEvent> > mEventList;
};

#endif /* CBASEMENU_H_ */
