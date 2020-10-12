#include "FileDialog.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>

int OpenFileDialog(char* filepath, int size)
{
	OPENFILENAME ofn;       // common dialog box structure
	HANDLE hf;              // file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filepath;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = size;
	ofn.lpstrFilter = "All\0*.*\0Nes\0*.nes\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return ((GetOpenFileNameA(&ofn) != 0) ? 1 : 0);
}

#endif

#ifdef PLATFORM_LINUX
//TODO https://stackoverflow.com/questions/18948783/c-simple-open-file-dialog-in-linux
#endif

