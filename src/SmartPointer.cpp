/*
 *  SmartPointer.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 10.08.10.
 *  code under LGPL
 *
 */

#include "SmartPointer.h"

#ifdef DEBUG
SDL_mutex *SmartPointer_CollMutex = NULL;
std::map< void *, SDL_mutex * > * SmartPointer_CollisionDetector = NULL;
#endif


template <> void SmartPointer_ObjectDeinit<SDL_Surface>( SDL_Surface * obj )
{
	SDL_FreeSurface(obj);
}

