/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "driveMenu.h"
#include "file_browse.h"

//#include "iconTitle.h"

char titleName[32];

int screenMode = 0;

bool applaunch = false;

using namespace std;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	int pathLen;
	std::string filename;

	//iconTitleInit();
	
	snprintf(titleName, sizeof(titleName), "GodMode9i v%i.%i.%i", 0, 1, 0);

	// Subscreen as a console
	videoSetMode(MODE_0_2D);
	vramSetBankG(VRAM_G_MAIN_BG);
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, true, true);

	if (!fatInitDefault()) {
		iprintf ("fatinitDefault failed!\n");
		stop();
	}

	keysSetRepeat(25,5);

	while(1) {

		if (screenMode == 0) {
			driveMenu();
		} else {
			filename = browseForFile();
		}

		if (applaunch) {
			// Construct a command line
			getcwd (filePath, PATH_MAX);
			pathLen = strlen (filePath);
			vector<char*> argarray;

			if ( strcasecmp (filename.c_str() + filename.size() - 5, ".argv") == 0) {

				FILE *argfile = fopen(filename.c_str(),"rb");
				char str[PATH_MAX], *pstr;
				const char seps[]= "\n\r\t ";

				while( fgets(str, PATH_MAX, argfile) ) {
					// Find comment and end string there
					if( (pstr = strchr(str, '#')) )
						*pstr= '\0';

					// Tokenize arguments
					pstr= strtok(str, seps);

					while( pstr != NULL ) {
						argarray.push_back(strdup(pstr));
						pstr= strtok(NULL, seps);
					}
				}
				fclose(argfile);
				filename = argarray.at(0);
			} else {
				argarray.push_back(strdup(filename.c_str()));
			}

			if ( strcasecmp (filename.c_str() + filename.size() - 4, ".nds") != 0 || argarray.size() == 0 ) {
				iprintf("no nds file specified\n");
			} else {
				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				iprintf ("Running %s with %d parameters\n", argarray[0], argarray.size());
				int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
				iprintf ("Start failed. Error %i\n", err);

			}

			while(argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}

			while (1) {
				swiWaitForVBlank();
				scanKeys();
				if (!(keysHeld() & KEY_A)) break;
			}
		}

	}

	return 0;
}
