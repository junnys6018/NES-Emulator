#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

// Opens a file dialog and puts the selected filename in filepath
// size specifies the size of the filepath buffer provided
// Returns 1 on success, 0 on failure
int OpenFileDialog(char* filepath, int size);

#endif // !FILE_DIALOG_H
