/*
 * COrderingInfo.h
 *
 *  Created on: 04.08.2009
 *      Author: gerstrong
 */

#ifndef CORDERINGINFO_H_
#define CORDERINGINFO_H_

#include <vector>
#include <string>
#include "CInfoScene.h"
#include "common/CMap.h"
#include "fileio/CExeFile.h"

class COrderingInfo : public CInfoScene
{
public:
	void init();
	void process();
	void teardown();
	
private:
	std::vector<std::string> m_Textline;
	
	int m_starty;			// start of y-coordinate in textheights
	int m_numberoflines;	// number of lines to print
	
	SmartPointer<CMap> mpMap;
	SmartPointer<SDL_Surface> mpTextSfc;
};

#endif /* CORDERINGINFO_H_ */
