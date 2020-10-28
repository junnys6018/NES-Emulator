#include "FileDialog.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>

int OpenFileDialog(char* filepath, int size)
{
	OPENFILENAMEA ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filepath;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = size;
	ofn.lpstrFilter = "All\0*.*\0Nes (.nes)\0*.nes\0";
	ofn.nFilterIndex = 2;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	return ((GetOpenFileNameA(&ofn) != 0) ? 0 : 1);
}

#endif

#ifdef PLATFORM_LINUX
//TODO https://stackoverflow.com/questions/18948783/c-simple-open-file-dialog-in-linux
#endif
