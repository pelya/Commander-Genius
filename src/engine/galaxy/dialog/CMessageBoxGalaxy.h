/*
 * CMessageBoxGalaxy.h
 *
 *  Created on: 30.03.2011
 *      Author: gerstrong
 */

#ifndef CMESSAGEBOXGALAXY_H_
#define CMESSAGEBOXGALAXY_H_

#include "CVec.h"
#include <string>
#include <SDL.h>
#include "SmartPointer.h"

class CMessageBoxGalaxy
{
public:

	/**
	 * \brief This constructor creates a typical Keen 4 Message Box
	 * \param Text 				Message to be shown
	 */
	CMessageBoxGalaxy(const std::string& Text);
	virtual ~CMessageBoxGalaxy() {};

	virtual void init();

	virtual void process();

	const bool isFinished() const
	{	return mMustClose;	}


protected:

	void initGalaxyFrame();
	void initText(const SDL_Rect &rect);


	bool mMustClose;
	SDL_Rect mMBRect;
	std::string mText;
	SmartPointer<SDL_Surface> mpMBSurface;
	unsigned int mTextHeight;
};

#endif /* CMESSAGEBOXGALAXY_H_ */
