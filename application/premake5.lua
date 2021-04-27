project "NES-Emulator" 
	kind "ConsoleApp"
	language "C"

	targetdir ("%{wks.location}/bin/" .. outputdir)
	objdir ("%{wks.location}/bin-int/" .. outputdir)

	files 
	{
		"src/**.h",
		"src/**.c",
		"vendor/cJSON/cJSON.c"
	}

	links 
	{
		"SDL2",
		"SDL2main",
		"glad",
		"core"
	}

	includedirs
	{
		"src",
		"../core/src",
		"vendor/SDL2-2.0.12/include",
		"vendor/stb",
		"vendor/cJSON",
		"vendor/glad/include"
	}
	
	filter "configurations:Debug"
		optimize "Debug"

	filter "configurations:Release"
		optimize "On"

	filter "system:windows"
		defines { "_CRT_SECURE_NO_WARNINGS", "PLATFORM_WINDOWS" }
		files
		{
			"res/NES-Emulator.rc"
		}
		libdirs 
		{
			"vendor/SDL2-2.0.12/lib/x64"
		}

	filter {"system:windows", "configurations:Release"}
		postbuildcommands
		{
			"copy \"%{wks.location}bin\\" .. outputdir .. "\\%{prj.name}.exe\" \"%{prj.location}%{prj.name}.exe\""
		}

	filter "system:linux"
		defines { "PLATFORM_LINUX" }
		links { "m" } -- Link with math library

	filter {"system:linux", "configurations:Release"}
		postbuildcommands
		{
			"@cp %{wks.location}/bin/" .. outputdir .. "/%{prj.name} %{prj.location}/%{prj.name}.out"
		}
