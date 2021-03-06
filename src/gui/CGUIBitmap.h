/*
 * CGUIBitmap.h
 *
 *  Created on: 22.04.2012
 *      Author: gerstrong
 */

#ifndef CGUIBITMAP_H_
#define CGUIBITMAP_H_

#include "CGUIControl.h"
#include "SmartPointer.h"
#include "graphics/CBitmap.h"
#include <string>

class CGUIBitmap : public CGUIControl
{
public:

	// Loads an Bitmap that is given directly
	CGUIBitmap(const SmartPointer<CBitmap>& pBitmap);

	// Load an Bitmap using an internal string ID of all the loaded Bitmaps
	CGUIBitmap(const std::string &text);


	void processLogic();

	void processRender(const CRect<float> &RectDispCoordFloat);

private:

	SmartPointer<CBitmap> mpBitmap;

};

#endif /* CGUIBITMAP_H_ */
