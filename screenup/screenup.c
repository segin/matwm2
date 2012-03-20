/* screenup.c: change orientation to upright, to fix what those
 * annoying kids keep screwing with. damn kids.
 */

#include <windows.h>

#ifndef DMDO_DEFAULT
#define DMDO_DEFAULT            0
#define DMDO_90                 1
#define DMDO_180                2
#define DMDO_270                3
#endif

int main(int argc, char **argv)
{
	DEVMODE dm;
	DWORD dwTemp;
	DWORD *orientation;
	
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	
	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
		/* Due to issues in win32 headers in tcc, we make pointer
		 * to what would be .dmDisplayOrientation
		 */
		orientation = (DWORD *) &dm.dmScale;
		switch (*orientation) 
		{
		case DMDO_270:
		case DMDO_90:
			dwTemp = dm.dmPelsHeight;
			dm.dmPelsHeight= dm.dmPelsWidth;
			dm.dmPelsWidth = dwTemp;
		default:
			*orientation = DMDO_DEFAULT;
			break;
		}
		if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings(&dm, 0)) {
			printf("ERROR: Cannot change orientation!\n");
		}
	} else {
		printf("ERROR: Cannot get display properties to change orientation!\n");
		exit(1);
	}
	return 0;
}