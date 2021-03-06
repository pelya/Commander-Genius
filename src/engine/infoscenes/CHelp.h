/*
 * CHelp.h
 *
 *  Created on: 16.11.2009
 *      Author: gerstrong
 */

#ifndef CHELP_H_
#define CHELP_H_

#include "CInfoScene.h"
#include "common/CMap.h"
#include "dialog/CTextViewer.h"
#include "fileio/CExeFile.h"
#include "SmartPointer.h"
#include <string.h>

class CHelp : public CInfoScene
{
public:
	CHelp(const std::string &type);

	void init();
	void teardown();

	void process();

private:
	std::string mTextType;
	SmartPointer<CTextViewer> mpTextViewer;
};

#endif /* CHELP_H_ */
