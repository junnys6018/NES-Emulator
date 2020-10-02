#include "string_util.h"
#include <string.h>

char* GetFileName(const char* filepath)
{
	return strrchr(filepath, '/') + 1;
}