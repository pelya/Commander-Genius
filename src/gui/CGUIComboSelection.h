/*
 * CGUIComboSelection.h
 *
 *  Created on: 26.02.2012
 *      Author: gerstrong
 */

#ifndef CGUICOMBOSELECTION_H_
#define CGUICOMBOSELECTION_H_

#include <string>
#include "CGUIControl.h"
#include "engine/CEvent.h"
#include "SmartPointer.h"


class CGUIComboSelection : public CGUIControl
{
public:

	CGUIComboSelection(	const std::string& text,
			const std::list<std::string>& optionsList);

	void setupButtonSurface(const std::string &optionText);

	const std::string& getSelection();

	void cycleOption();

	bool sendEvent(const InputCommands command);

	void setSelection( const std::string& selectionText );

	void setList(const char **strArray, const int numElem);

	void processLogic();

	void drawNoStyle(SDL_Rect& lRect);

	virtual void drawVorticonStyle(SDL_Rect& lRect);

	void drawGalaxyStyle(SDL_Rect& lRect);

	virtual void processRender(const CRect<float> &RectDispCoordFloat);

protected:

	std::string mText;
	std::list<std::string> mOptionsList;
	std::list<std::string>::const_iterator mOLCurrent;

	SmartPointer<SDL_Surface> mpTextDarkSfc;
	SmartPointer<SDL_Surface> mpTextLightSfc;
	SmartPointer<SDL_Surface> mpTextDisabledSfc;

	void (CGUIComboSelection::*drawButton)(SDL_Rect&);

};

#endif /* CGUICOMBOSELECTION_H_ */
