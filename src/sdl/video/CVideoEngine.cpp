/*
 * CVideoEngine.cpp
 *
 *  Created on: 05.02.2011
 *      Author: gerhard
 */

#include "sdl/CTimer.h"
#include "CVideoEngine.h"
#include "CLogFile.h"

CVideoEngine::CVideoEngine(const CVidConfig& VidConfig) :
BlitSurface(NULL),
FilteredSurface(NULL),
ScrollSurface(NULL),       // 512x512 scroll buffer
FXSurface(NULL),
m_VidConfig(VidConfig),
mSbufferx(0),
mSbuffery(0),
screen(NULL),
m_Mode(0)
{}


CVideoEngine::~CVideoEngine()
{}


bool CVideoEngine::init()
{
	const CRect<Uint16> &GameRect = m_VidConfig.m_GameRect;

	// Setup mode depends on some systems.
#if defined(CAANOO) || defined(WIZ) || defined(DINGOO) || defined(NANONOTE) || defined(ANDROID)
	m_Mode = SDL_SWSURFACE;
#elif defined(GP2X)
	m_Mode = SDL_HWSURFACE;
#else
	// Support for double-buffering
	m_Mode = SDL_HWPALETTE;
#endif

	// Enable OpenGL
#ifdef USE_OPENGL
	if(m_VidConfig.m_opengl)
	{
		if(m_VidConfig.vsync)
		{
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	#if SDL_VERSION_ATLEAST(1, 3, 0)
	#else
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	#endif
		}

		m_Mode |= SDL_OPENGL;
	}
	else
#endif
	{
		if(m_VidConfig.vsync)
		{
			m_Mode |= (SDL_DOUBLEBUF | SDL_HWSURFACE);
		}
	}


	// Now we decide if it will be fullscreen or windowed mode.
	if(m_VidConfig.Fullscreen)
		m_Mode |= SDL_FULLSCREEN;
	else
		m_Mode |= SDL_RESIZABLE;

	// And set the proper Display Dimensions
	// The screen is also setup in this function
	if( !resizeDisplayScreen(m_VidConfig.m_DisplayRect) )
		return false;

	// If Fullscreen hide the mouse cursor.
	// Anyway, it just can point but does not interact yet
 	SDL_ShowCursor(true);

 	m_src_slice = GameRect.w*screen->format->BytesPerPixel;

	return true;
}

SDL_Surface* CVideoEngine::createSurface( std::string name, bool alpha, int width, int height, int bpp, int mode, SDL_PixelFormat* format )
{
	SDL_Surface *temporary, *optimized;

	temporary = SDL_CreateRGBSurface( mode, width, height, bpp, format->Rmask, format->Gmask, format->Bmask, format->Amask);
	if (alpha && bpp==32)
		optimized = SDL_DisplayFormatAlpha( temporary );
	else
		optimized = SDL_DisplayFormat( temporary );

	SDL_FreeSurface(temporary);

	if (!optimized)
	{
		g_pLogFile->textOut(RED,"VideoDriver: Couldn't create surface:" + name +"<br>");
		return NULL;
	}

	bpp = optimized->format->BitsPerPixel;
	return optimized;
}



void CVideoEngine::blitScrollSurface() // This is only for tiles
{									   // The name should be changed
	SDL_Rect srcrect, dstrect;
	Sint16 sbufferx, sbuffery;
	const SDL_Rect Gamerect = m_VidConfig.m_GameRect.SDLRect();

	dstrect.x = 0; dstrect.y = 0;
	srcrect.x =	sbufferx = mSbufferx;
	srcrect.y = sbuffery = mSbuffery;

	const bool wraphoz = (sbufferx > (512-Gamerect.w));
	const bool wrapvrt = (sbuffery > (512-Gamerect.h));

	dstrect.w = (Gamerect.w>sbufferx) ? Gamerect.w-sbufferx : Gamerect.w ;
	dstrect.h = (Gamerect.h>sbuffery) ? Gamerect.h-sbuffery : Gamerect.h ;
	srcrect.w = wraphoz ? (512-sbufferx) : Gamerect.w;
	srcrect.h = wrapvrt ? (512-sbuffery) : Gamerect.h;

	SDL_BlitSurface(ScrollSurface, &srcrect, BlitSurface, &dstrect);

	const Uint16 saveDstw = dstrect.w;
	const Uint16 saveSrcw = srcrect.w;

	if (wraphoz)
	{
		dstrect.x = srcrect.w;
		dstrect.w = Gamerect.w - dstrect.x;
		srcrect.x = 0;
		srcrect.w = Gamerect.w - srcrect.w;
		SDL_BlitSurface(ScrollSurface, &srcrect, BlitSurface, &dstrect);

		if(!wrapvrt)
			return;
	}

	if (wrapvrt)
	{
		dstrect.y = srcrect.h;
		dstrect.h = Gamerect.h - dstrect.y;
		srcrect.y = 0;
		srcrect.h = Gamerect.h - srcrect.h;
		SDL_BlitSurface(ScrollSurface, &srcrect, BlitSurface, &dstrect);

		if(!wraphoz)
			return;
	}

	srcrect.x = sbufferx;
	srcrect.w = saveSrcw;
	dstrect.x = 0;
	dstrect.w = saveDstw;
	SDL_BlitSurface(ScrollSurface, &srcrect, BlitSurface, &dstrect);
}

void CVideoEngine::stop()
{
	g_pLogFile->textOut(GREEN, "Freeing the following graphical surfaces:<br>\n");

    if( BlitSurface )
    {
        SDL_FreeSurface(BlitSurface);
        g_pLogFile->textOut("freed BlitSurface<br>");
        BlitSurface=NULL;
    }

    if( FilteredSurface )
    {
        SDL_FreeSurface(FilteredSurface);
        g_pLogFile->textOut("freed FilteredSurface<br>");
        FilteredSurface = NULL;
    }

    if(ScrollSurface && (ScrollSurface->map != NULL))
    {
        SDL_FreeSurface(ScrollSurface);
        g_pLogFile->textOut("freed ScrollSurface<br>");
        ScrollSurface=NULL;
    }
    if(FXSurface)
    {
        SDL_FreeSurface(FXSurface);
        g_pLogFile->textOut("freed FXSurface<br>");
        FXSurface=NULL;
    }
}

void CVideoEngine::shutdown()
{
	stop();
}
