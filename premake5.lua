workspace "Emulation"
	architecture "x64"
	configurations {"Debug", "Release"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}/%{prj.name}"

include "application/vendor/glad"
include "core"
include "application"