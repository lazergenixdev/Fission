workspace 'Fission'

	architecture "x86_64"

	configurations { 'Debug', 'Release', 'Dist' }

	
    flags { 'MultiProcessorCompile', 'MFC' }
    defines '_CRT_SECURE_NO_WARNINGS'
	
	startproject 'sandbox'

	
    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
    IncludeDir = {}
    IncludeDir["yaml"] = "%{wks.location}/Fission/vendor/yaml-cpp/include"
    IncludeDir["lazerlib"] = "%{wks.location}/Fission/vendor/lazerlib"
    IncludeDir["imgui"] = "%{wks.location}/Fission/vendor/imgui"


group "Dependencies"
    include "Fission/vendor/yaml-cpp"
    include "Fission/vendor/lazerlib"
--    include "Fission/vendor/imgui"
group ""

include 'Fission'
include 'sandbox'