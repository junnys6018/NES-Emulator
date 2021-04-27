project "core"
	kind "StaticLib"
	language "C"

	targetdir ("%{wks.location}/bin/" .. outputdir)
	objdir ("%{wks.location}/bin-int/" .. outputdir)

	files 
	{
		"src/**.h",
		"src/**.c",
	}

	includedirs
	{
		"src/",
	}
	
	filter "configurations:Debug"
		optimize "Debug"

	filter "configurations:Release"
		optimize "On"

	filter "system:windows"
		defines { "_CRT_SECURE_NO_WARNINGS", "PLATFORM_WINDOWS" }

	filter "system:linux"
		defines { "PLATFORM_LINUX" }
		links { "m" } -- Link with math library