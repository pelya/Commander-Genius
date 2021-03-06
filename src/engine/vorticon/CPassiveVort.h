/*
 * CPassiveVort.h
 *
 *  Created on: 07.03.2010
 *      Author: gerstrong
 */

#ifndef CPASSIVEVORT_H_
#define CPASSIVEVORT_H_

#include "engine/CPassive.h"
#include "common/CBehaviorEngine.h"
#include "engine/CIntro.h"

namespace vorticon
{

class CPassiveVort : public CPassive
{
public:

	bool init(char mode = INTRO);

	void process();

private:
	SmartPointer<CIntro> mpIntroScreen;
	SmartPointer<CTitle> mpTitleScreen;
	SmartPointer<CMap> mpMap;
};

}

#endif /* CPASSIVEVORT_H_ */
