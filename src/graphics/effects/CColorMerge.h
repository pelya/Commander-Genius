/*
 * CColorMerge.h
 *
 *  Created on: 14.12.2009
 *      Author: gerstrong
 *
 * This class gets two surface and blends them the way
 * the player will see a smooth scene transition.
 * NOTE: Rember to always use the Source and TargetSurfaces
 * If you don't the effect won't work!
 */

#ifndef CCOLORMERGE_H_
#define CCOLORMERGE_H_

#include <SDL.h>

#include "CEffects.h"
#include "SmartPointer.h"
#include "graphics/CBitmap.h"
#include "sdl/CTimer.h"


class CColorMerge : public CEffects
{
public:
	CColorMerge(Uint8 speed);

	void process();

private:
	void getSnapshot();

	Uint8 m_Speed;
	Uint8 m_Alpha;
	CTimer mTimer;

	SmartPointer<SDL_Surface> mpOldSurface;
};

#endif /* CCOLORMERGE_H_ */
