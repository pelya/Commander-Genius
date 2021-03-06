/*
 * CStatusScreenGalaxyEp4.cpp
 *
 *  Created on: 13.03.2011
 *      Author: gerstrong
 */

#include "CStatusScreenGalaxyEp4.h"
#include "graphics/CGfxEngine.h"
#include "common/CBehaviorEngine.h"
#include "StringUtils.h"

CStatusScreenGalaxyEp4::CStatusScreenGalaxyEp4(const stItemGalaxy& Item, const std::string &LevelName) :
CStatusScreenGalaxy(Item, LevelName)
{}


void CStatusScreenGalaxyEp4::GenerateStatus()
{
	SDL_Rect EditRect;
	drawBase(EditRect);

	CFont &Font = g_pGfxEngine->getFont(0);
	Font.setupColor(0x555555);

	Font.drawFontCentered(mpStatusSurface.get(), "LOCATION", EditRect.x, EditRect.w, EditRect.y, false);

	// Temporary Rect for drawing some stuff like background for scores and so...
	SDL_Rect TempRect;

	// Location Rect
	TempRect.x = EditRect.x;
	TempRect.y = EditRect.y+10;
	TempRect.w = EditRect.w;
	TempRect.h = 20;

	Font.setupColor(0x0);
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFFFFFFFF);
	Font.drawFontCentered(mpStatusSurface.get(), m_LevelName, TempRect.x, TempRect.w, TempRect.y+6, false);
	Font.setupColor(0x444444);


	/// SCORE and EXTRA Rect
	TempRect.x = EditRect.x;
	TempRect.y = EditRect.y+32;
	TempRect.w = EditRect.w/2; TempRect.h = 10;
	Font.drawFont(mpStatusSurface.get(), "SCORE            EXTRA", TempRect.x+16, TempRect.y);
	TempRect.w = (EditRect.w/2)-16; TempRect.h = 11;
	TempRect.y = EditRect.y+42;

	// Score Box
	TempRect.w = 8*8;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	g_pGfxEngine->drawDigits(getRightAlignedString(itoa(m_Item.m_points), 8), TempRect.x, TempRect.y+2, mpStatusSurface.get());

	// Extra Box
	TempRect.x = EditRect.x+96;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	g_pGfxEngine->drawDigits(getRightAlignedString(itoa(m_Item.m_lifeAt), 8), TempRect.x, TempRect.y+2, mpStatusSurface.get());

	/// RESCUED and LEVEL Rects
	TempRect.x = EditRect.x;
	TempRect.y = EditRect.y+56;
	TempRect.w = EditRect.w/2; TempRect.h = 10;
	Font.drawFont(mpStatusSurface.get(), "RESCUED           LEVEL", TempRect.x+8, TempRect.y);
	TempRect.w = (EditRect.w/2)-16; TempRect.h = 11;
	TempRect.y = EditRect.y+66;

	// Rescued Box
	TempRect.w = 8*8;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	for( int count=0 ; count<m_Item.m_special.ep4.elders ; count++ )
		g_pGfxEngine->drawDigit(40, TempRect.x+8*count, TempRect.y+1, mpStatusSurface.get());

	// Level Box
	TempRect.x = EditRect.x+96;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFFFFFFFF);
	Font.setupColor(0x0);
	std::string difftext;
	if(g_pBehaviorEngine->mDifficulty == EASY)
		difftext = "Easy";
	else if(g_pBehaviorEngine->mDifficulty == NORMAL)
		difftext = "Normal";
	else if(g_pBehaviorEngine->mDifficulty == HARD)
		difftext = "Hard";
	else
		difftext = "???";
	Font.drawFontCentered(mpStatusSurface.get(), difftext, TempRect.x, TempRect.w, TempRect.y+1, false);
	Font.setupColor(0x333333);

	// Keys Box
	TempRect.x = EditRect.x;
	TempRect.y = EditRect.y+80;
	Font.drawFont(mpStatusSurface.get(), "KEYS", TempRect.x, TempRect.y);
	TempRect.w = 8*4; TempRect.h = 10;
	TempRect.x = TempRect.x+8*5;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	if(m_Item.m_gem.red)
		g_pGfxEngine->drawDigit(36, TempRect.x, TempRect.y+1, mpStatusSurface.get());
	if(m_Item.m_gem.yellow)
		g_pGfxEngine->drawDigit(37, TempRect.x+8, TempRect.y+1, mpStatusSurface.get());
	if(m_Item.m_gem.blue)
		g_pGfxEngine->drawDigit(38, TempRect.x+16, TempRect.y+1, mpStatusSurface.get());
	if(m_Item.m_gem.green)
		g_pGfxEngine->drawDigit(39, TempRect.x+24, TempRect.y+1, mpStatusSurface.get());

	// Ammo Box
	TempRect.x = EditRect.x+96;
	TempRect.y = EditRect.y+80;
	Font.drawFont(mpStatusSurface.get(), "AMMO", TempRect.x, TempRect.y);
	TempRect.w = 8*3; TempRect.h = 10;
	TempRect.x = TempRect.x+8*5;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	g_pGfxEngine->drawDigits(getRightAlignedString(itoa(m_Item.m_bullets), 3), TempRect.x, TempRect.y+1, mpStatusSurface.get());

	// Keens Box
	TempRect.x = EditRect.x;
	TempRect.y = EditRect.y+96;
	Font.drawFont(mpStatusSurface.get(), "KEENS", TempRect.x, TempRect.y);
	TempRect.w = 8*2; TempRect.h = 10;
	TempRect.x = TempRect.x+8*5+8;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	g_pGfxEngine->drawDigits(getRightAlignedString(itoa(m_Item.m_lifes), 2), TempRect.x, TempRect.y+1, mpStatusSurface.get());

	// Drops Box
	TempRect.x = EditRect.x+96;
	TempRect.y = EditRect.y+96;
	Font.drawFont(mpStatusSurface.get(), "DROPS", TempRect.x, TempRect.y);
	TempRect.w = 8*2; TempRect.h = 10;
	TempRect.x = TempRect.x+8*5+8;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFF000000);
	g_pGfxEngine->drawDigits(getRightAlignedString(itoa(m_Item.m_drops), 2), TempRect.x, TempRect.y+1, mpStatusSurface.get());

	// Swim Suit Box
	TempRect.x = EditRect.x;
	TempRect.y = EditRect.y+114;
	TempRect.w = (EditRect.w/2)-16; TempRect.h = 11;
	SDL_FillRect(mpStatusSurface.get(), &TempRect, 0xFFFFFFFF);
	Font.setupColor(0x0);
	Font.drawFontCentered(mpStatusSurface.get(), m_Item.m_special.ep4.swimsuit ? "Swim Suit" : "???", TempRect.x, TempRect.w, TempRect.y+1, false);

	// Press a Key Sign
	CTilemap &Tilemap = g_pGfxEngine->getTileMap(2);
	TempRect.x = EditRect.x+(EditRect.w/2);
	TempRect.y = EditRect.y+110;
	for( int c=0 ; c<10 ; c++ )
		Tilemap.drawTile(mpStatusSurface.get(), TempRect.x+c*8, TempRect.y, 72+c);
	TempRect.y += 8;
	for( int c=0 ; c<10 ; c++ )
		Tilemap.drawTile(mpStatusSurface.get(), TempRect.x+c*8, TempRect.y, 82+c);

}
