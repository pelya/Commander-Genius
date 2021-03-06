/* MAIN.C
 This is CloneKeen's main source file.

 The CloneKeen source may be freely distributed and
 modified as per the GPL but please give credit to
 the original author, Caitlin Shaw and to the new author
 Gerstrong.

 If you make any changes or improvements to the code that
 you feel merit inclusion in the source tree email them
 to me at gerstrong@gmail.com or get my latest email
 from the SourceForge site.

 Thanks to ID Software for the "Commander Keen: Invasion of
 the Vorticons" games. "Commander Keen" and it's associated
 graphics, level, and sound files are the property of ID
 Software. Commander Genius requires the original version of a
 Commander Keen game in order to be able to interpret that
 episode.

 Enjoy the Code
 - The Commander Genius Team

 CloneKeen          2003-2005   Caitlin Shaw
 CloneKeenPlus      2008-2012   Gerstrong
 Commander Genius   2009-2012   Tulip, Pickle, DaVince, Albert and Pizza2004
 Commander Genius   2010-2012   Lemm, Commander Spleen, Zear, Pelya and FCTW
 */

#ifdef ANDROID
	#include <SDL_main.h>
#endif

#include "CLogFile.h"
#include "CGame.h"
#include "FindFile.h"
#include "ThreadPool.h"

/**
 * \brief  This is the function where CG beings
 *
 * \param	argc   	number of arguments
 * \param	argv   	pointer to  char arrays where
 * 					where the passed arguments are stored
 * 					in the process
 * \return	        This always returns 0. If
 * 					some errors appear, take a look
 * 					at the Logfile.
 *
 */
int main(int argc, char *argv[])
{
	// Check if CG should look into a given directory
	if(argc >= 1)
	{
		binary_dir = argv[0];
		size_t slashpos = findLastPathSep(binary_dir);
		
		if(slashpos != std::string::npos)
		{
			binary_dir.erase(slashpos);
			binary_dir = SystemNativeToUtf8(binary_dir);
		}
		else
		{
			binary_dir = ".";
		}
	}
	else
	{
		warnings << "Binary-argument not given, assuming current dir" << endl;
		binary_dir = ".";
	}

	binary_dir = GetAbsolutePath(binary_dir);

	InitThreadPool();
	InitSearchPaths();

	g_pLogFile->CreateLogfile("CGLog.html");

	// The Game Class instance is the main class managing whole
	// interpreter instance. TODO: It should be a singleton
	CGame Game;
	
	////////////////////////////
	// Initialize Game Engine //
	////////////////////////////
	if( Game.init( argc, argv ) )
	{
		///////////////////////
		// Start Game Engine //
		///////////////////////
		Game.run();
	}

	/////////////////////////
	// Cleanup Game Engine //
	/////////////////////////
	//Game.cleanup();



	std::cout << "Thank you very much for playing this game!" << std::endl;

	UnInitThreadPool();
	return 0;
}
