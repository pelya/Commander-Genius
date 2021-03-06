/*
 * CTantalusRay.h
 *
 *  Created on: 21.12.2009
 *      Author: gerstrong
 */

#ifndef CTANTALUSRAY_H_
#define CTANTALUSRAY_H_

#include "CFinale.h"
#include "engine/vorticon/dialog/CMessageBoxVort.h"
#include "engine/vorticon/CVorticonSpriteObject.h"
#include "common/CMap.h"
#include "engine/vorticon/ai/CVorticonSpriteObjectAI.h"
#include "graphics/CBitmap.h"
#include "sdl/CVideoDriver.h"

class CTantalusRay : public CFinale
{
public:
	CTantalusRay(std::list< SmartPointer<CMessageBoxVort> > &messageBoxes,
				const SmartPointer<CMap> &pMap,
				std::vector<CVorticonSpriteObject*> &vect_obj,
				SmartPointer<CVorticonSpriteObjectAI> &objectai);

	void process();

	void shootray();
	void explodeEarth();

private:
	bool m_mustsetup;
	int m_alternate_sprite;
	CMessageBox *mp_MessageBox;
	CVorticonSpriteObject *mp_ShootObject;
	SmartPointer<CVorticonSpriteObjectAI> mObjectAI;
	Uint32 m_timer;
	CBitmap *mp_Bitmap;

	int shot_x, shot_y;

	void (CTantalusRay::*mp_process)();
};

#endif /* CTANTALUSRAY_H_ */
