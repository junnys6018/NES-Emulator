workspace "Emulation"
	architecture "x64"
	configurations {"Debug", "Release"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

project "NES-Emulator" 
kind "ConsoleApp"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"src/**.h",
		"src/**.c",
		"tests/**.c",
		"tests/**.h",
		"vendor/cJSON/cJSON.c"
	}

	links 
	{
		"SDL2",
		"SDL2main",
	}

	includedirs
	{
		"src",
		"tests",
		"vendor/SDL2-2.0.12/include",
		"vendor/stb",
		"vendor/cJSON"
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
			"copy \"%{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\%{prj.name}.exe\" \"%{prj.location}%{prj.name}.exe\""
		}

	filter "system:linux"
		defines { "PLATFORM_LINUX" }
		links { "m" } -- Link with math library

	filter {"system:linux", "configurations:Release"}
		postbuildcommands
		{
			"@cp %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/%{prj.name} %{prj.location}/%{prj.name}.out"
		}