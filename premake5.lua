workspace "Emulation"
	architecture "x64"
	configurations {"Debug", "Release"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

project "Emulation" 
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
	}

	links 
	{
		"vendor/SDL2",
		"vendor/SDL2main",
	}

	includedirs
	{
		"vendor/SDL2-2.0.12/include",
		"src",
		"tests",
		"vendor/stb",
	}
	
	filter "configurations:Debug"
		optimize "Debug"

	filter "configurations:Release"
		optimize "On"

	filter "system:windows"
		defines { "_CRT_SECURE_NO_WARNINGS", "PLATFORM_WINDOWS" }

	filter {"system:windows", "configurations:Release"}
		postbuildcommands
		{
			"copy \"%{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\%{prj.name}.exe\" \"%{prj.location}%{prj.name}.exe\""
		}

	filter "system:linux"
		defines { "PLATFORM_LINUX" }

	filter {"system:linux", "configurations:Release"}
		postbuildcommands
		{
			"@cp %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/%{prj.name} %{prj.location}/%{prj.name}.out"
		}