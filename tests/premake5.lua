project "tests" 
	kind "ConsoleApp"
	language "C"

	targetdir ("%{wks.location}/bin/" .. outputdir)
	objdir ("%{wks.location}/bin-int/" .. outputdir)

	files 
	{
		"src/**.h",
		"src/**.c",
		"../application/src/Util/timer.c", -- Include timer from application project
		"../application/src/Util/timer.h",
	}

	links 
	{
		"core"
	}

	includedirs
	{
		"src",
		"../core/src",
		"../application/src/Util"
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
