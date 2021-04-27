#include "StartupOptions.h"

#include <stdio.h>
#include <string.h>

#include <cJSON.h>

static StartupOptions rts;
static const char* const default_json_file = \
"{\n\
	// For key, use a Key Name from the table listed here https://wiki.libsdl.org/SDL_Scancode \n\
	\"controls\": [\n\
		{\n\
			\"button\": \"A\",\n\
			\"key\": \"X\"\n\
		},\n\
		{\n\
			\"button\": \"B\",\n\
			\"key\": \"Z\"\n\
		},\n\
		{\n\
			\"button\": \"Start\",\n\
			\"key\": \"RETURN\"\n\
		},\n\
		{\n\
			\"button\": \"Select\",\n\
			\"key\": \"TAB\"\n\
		},\n\
		{\n\
			\"button\": \"Up\",\n\
			\"key\": \"UP\"\n\
		},\n\
		{\n\
			\"button\": \"Down\",\n\
			\"key\": \"DOWN\"\n\
		},\n\
		{\n\
			\"button\": \"Left\",\n\
			\"key\": \"LEFT\"\n\
		},\n\
		{\n\
			\"button\": \"Right\",\n\
			\"key\": \"RIGHT\"\n\
		}\n\
	],\n\
\n\
	\"fullscreenOnStartup\": false,\n\
	\"startupWindowSize\": {\n\
		\"width\": 1305,\n\
		\"height\": 738\n\
	},\n\
\n\
	\"fontSize\": 15,\n\
	\"fontStyle\": \"Consola.ttf\"\n\
}";

StartupOptions* GetStartupOptions()
{
	return &rts;
}

void UseDefaultRuntimeSettings()
{
	rts.key_A = SDL_SCANCODE_X;
	rts.key_B = SDL_SCANCODE_Z;
	rts.key_start = SDL_SCANCODE_RETURN;
	rts.key_select = SDL_SCANCODE_TAB;
	rts.key_up = SDL_SCANCODE_UP;
	rts.key_down = SDL_SCANCODE_DOWN;
	rts.key_left = SDL_SCANCODE_LEFT;
	rts.key_right = SDL_SCANCODE_RIGHT;

	rts.fullscreen_on_startup = false;

	rts.startup_width = 1305;
	rts.startup_height = 738;

	rts.font_size = 15;
	strcpy(rts.font_style, "Consola.ttf");
}

void OnError(char* message)
{
	printf("[ERROR] Could not parse settings.json (%s)\n", message);
	printf("[INFO] Using default settings\n");

	UseDefaultRuntimeSettings();
}

void LoadStartupOptions()
{
	FILE* file = fopen("settings.json", "r");
	if (!file)
	{
		printf("[WARN] settings.json could not be found. Generating settings file\n");

		file = fopen("settings.json", "w");

		fwrite(default_json_file, 1, strlen(default_json_file), file);

		fclose(file);

		UseDefaultRuntimeSettings();
		return;
	}

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = malloc(size);
	fread(buffer, 1, size, file);

	// Remove comments, cJSON does not recognise comments and fails to parse if comments exist
	char* comment;
	while (comment = strstr(buffer, "//"))
	{
		char* newline = strstr(comment, "\r\n");
		if (!newline)
			newline = strchr(comment, '\n');
		if (!newline)
			newline = buffer + size; // eof

		// fill with spaces
		memset(comment, ' ', newline - comment);
	}

	cJSON* root = cJSON_ParseWithLength(buffer, size);

	fclose(file);
	free(buffer);

	char* error_msg = NULL;

	cJSON* controls = cJSON_GetObjectItemCaseSensitive(root, "controls");
	if (!controls)
	{
		error_msg = "no controls listed";
		goto done;
	}

	cJSON* control;
	cJSON_ArrayForEach(control, controls)
	{
		cJSON* button = cJSON_GetObjectItemCaseSensitive(control, "button");
		cJSON* key = cJSON_GetObjectItemCaseSensitive(control, "key");
		if (!button || !key)
		{
			error_msg = "missing button or key";
			goto done;
		}

		if (SDL_GetScancodeFromName(key->valuestring) == SDL_SCANCODE_UNKNOWN)
		{
			error_msg = "invalid key name";
			goto done;
		}

		if (strcmp(button->valuestring, "A") == 0)
		{
			rts.key_A = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "B") == 0)
		{
			rts.key_B = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "Start") == 0)
		{
			rts.key_start = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "Select") == 0)
		{
			rts.key_select = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "Up") == 0)
		{
			rts.key_up = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "Down") == 0)
		{
			rts.key_down = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "Left") == 0)
		{
			rts.key_left = SDL_GetScancodeFromName(key->valuestring);
		}
		else if (strcmp(button->valuestring, "Right") == 0)
		{
			rts.key_right = SDL_GetScancodeFromName(key->valuestring);
		}
		else
		{
			error_msg = "invalid button name";
			goto done;
		}
	}

	cJSON* fullscreen_on_startup = cJSON_GetObjectItemCaseSensitive(root, "fullscreenOnStartup");
	if (!fullscreen_on_startup)
	{
		error_msg = "missing fullscreenOnStartup setting";
		goto done;
	}

	if (cJSON_IsBool(fullscreen_on_startup))
	{
		rts.fullscreen_on_startup = (fullscreen_on_startup->type == cJSON_True);
	}
	else
	{
		error_msg = "fullscreenOnStartup not a bool";
		goto done;
	}

	cJSON* startup_window_size = cJSON_GetObjectItemCaseSensitive(root, "startupWindowSize");
	if (!startup_window_size)
	{
		error_msg = "missing startupWindowSize";
		goto done;
	}

	cJSON* width = cJSON_GetObjectItemCaseSensitive(startup_window_size, "width");
	cJSON* height = cJSON_GetObjectItemCaseSensitive(startup_window_size, "height");
	if (!width || !height)
	{
		error_msg = "startupWindowSize width or height invalid";
		goto done;
	}
	else if (!cJSON_IsNumber(width) || !cJSON_IsNumber(height))
	{
		error_msg = "width or height not a number";
		goto done;
	}

	rts.startup_width = width->valueint;
	rts.startup_height = height->valueint;

	cJSON* fontsize = cJSON_GetObjectItemCaseSensitive(root, "fontSize");
	if (!fontsize)
	{
		error_msg = "fontSize setting invalid";
		goto done;
	}
	if (!cJSON_IsNumber(fontsize))
	{
		error_msg = "fontSize not a number";
		goto done;
	}
	rts.font_size = fontsize->valueint;

	cJSON* fontstyle = cJSON_GetObjectItemCaseSensitive(root, "fontStyle");
	if (!fontstyle)
	{
		error_msg = "fontstyle setting invalid";
		goto done;
	}
	if (!cJSON_IsString(fontstyle))
	{
		error_msg = "fontstyle not a string";
		goto done;
	}
	strcpy(rts.font_style, fontstyle->valuestring);

done:
	if (error_msg)
	{
		OnError(error_msg);
	}
	cJSON_Delete(root);
}
