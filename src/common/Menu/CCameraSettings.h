/*
 * CCameraSettings.h
 *
 *  Created on: 22.05.2010
 *      Author: gerstrong
 */

#ifndef CCAMERASETTINGS_H_
#define CCAMERASETTINGS_H_

#include "CBaseMenu.h"
#include "sdl/CVideoDriver.h"
#include "gui/CGUINumberControl.h"


class CCameraSettings : public CBaseMenu
{
public:
	CCameraSettings();

	void init();

	void release();

private:
	st_camera_bounds m_CameraBounds;
	int m_changeint;

	CGUINumberControl	*mpLeftAdjust;
	CGUINumberControl	*mpRightAdjust;
	CGUINumberControl	*mpTopAdjust;
	CGUINumberControl	*mpBottomAdjust;
	CGUINumberControl	*mpSpeedAdjust;
};

#endif /* CCAMERASETTINGS_H_ */
