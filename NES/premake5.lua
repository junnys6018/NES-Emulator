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
	}

	links 
	{
		"vendor/SDL2", "vendor/SDL2main"
	}

	includedirs
	{
		"vendor/SDL2-2.0.12/include"
	}
