workspace "Web-Build"
	configurations {"Release"}

project "Web-Build"
	kind "StaticLib"
	language "C"

	targetdir ("%{wks.location}/bin/")
	objdir ("%{wks.location}/bin-int/")

	files 
	{
		"../core/src/**.h",
		"../core/src/**.c",
		"api.c"
	}

	includedirs
	{
		"../core/src/",
	}

	filter "configurations:Release"
		optimize "On"
